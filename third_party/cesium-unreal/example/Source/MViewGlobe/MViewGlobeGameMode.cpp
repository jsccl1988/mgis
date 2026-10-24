// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "MViewGlobeGameMode.h"

#include "Cesium3DTileset.h"
#include "CesiumGeoreference.h"
#include "CesiumIonRasterOverlay.h"
#include "CesiumSunSky.h"
#include "CesiumUrlTemplateRasterOverlay.h"
#include "CesiumVectorStyle.h"
#include "CesiumVectorTilesRasterOverlay.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "Kismet/GameplayStatics.h"
#include "MViewCameraPawn.h"
#include "MViewGeologyDemo.h"
#include "MViewGroundwaterDemo.h"
#include "MViewRiverFloodDemo.h"
#include "MViewWindFieldDemo.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"

namespace
{
// Google Maps Platform Photorealistic 3D Tiles via Cesium ion.
constexpr int64 kGooglePhotorealisticAssetId = 2275207;
// Cesium World Terrain (used only when CESIUM_ION_ASSET overrides away from Google).
constexpr int64 kDefaultIonImageryAssetId = 2;
// MapLibre demotiles (TileJSON / MVT) — reachable when Esri / OSM often are not.
constexpr TCHAR kDefaultMapLibreTilesJson[] =
	TEXT("https://demotiles.maplibre.org/tiles/tiles.json");

void MViewDemoConsoleHandler(const TArray<FString>& Args)
{
	if (Args.Num() < 1 || !GEngine)
	{
		UE_LOG(LogTemp, Warning,
			   TEXT("Usage: MView.Demo globe|earth|groundwater|wind|geology|flood"));
		return;
	}
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (!World || !World->IsGameWorld())
		{
			continue;
		}
		if (AMViewGlobeGameMode* GameMode = World->GetAuthGameMode<AMViewGlobeGameMode>())
		{
			GameMode->Demo(Args[0]);
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("MView.Demo: no MViewGlobeGameMode in play world"));
}

static FAutoConsoleCommand GMVDemoCommand(
	TEXT("MView.Demo"),
	TEXT("Switch view: globe|earth|groundwater|wind|geology|flood"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&MViewDemoConsoleHandler));
}  // namespace


AMViewGlobeGameMode::AMViewGlobeGameMode()
{
	DefaultPawnClass = AMViewCameraPawn::StaticClass();
	PlayerControllerClass = APlayerController::StaticClass();
}

void AMViewGlobeGameMode::InitGame(const FString& MapName, const FString& Options,
								   FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	ResolveDemoKind(Options);
	UE_LOG(LogTemp, Display, TEXT("MViewGlobeGameMode demo kind=%s"),
		   MViewDemoKindName(ActiveKind));
}

void AMViewGlobeGameMode::StartPlay()
{
	Super::StartPlay();
	SpawnActiveDemo();
}

void AMViewGlobeGameMode::Demo(const FString& Name)
{
	EMViewDemoKind Parsed;
	if (!MViewTryParseDemoKind(Name, Parsed))
	{
		UE_LOG(LogTemp, Warning,
			   TEXT("MView.Demo expected globe|earth|groundwater|wind|geology|flood, got '%s'"),
			   *Name);
		return;
	}
	ActiveKind = Parsed;
	SpawnActiveDemo();
}

void AMViewGlobeGameMode::ResolveDemoKind(const FString& Options)
{
	FString FromOptions;
	if (FParse::Value(*Options, TEXT("MViewDemo="), FromOptions) ||
		FParse::Value(*Options, TEXT("Demo="), FromOptions))
	{
		EMViewDemoKind Parsed;
		if (MViewTryParseDemoKind(FromOptions, Parsed))
		{
			ActiveKind = Parsed;
			return;
		}
	}

	FString FromCmd;
	if (FParse::Value(FCommandLine::Get(), TEXT("-MViewDemo="), FromCmd) ||
		FParse::Value(FCommandLine::Get(), TEXT("-Demo="), FromCmd))
	{
		EMViewDemoKind Parsed;
		if (MViewTryParseDemoKind(FromCmd, Parsed))
		{
			ActiveKind = Parsed;
		}
	}
}

void AMViewGlobeGameMode::DestroyGlobeActors()
{
	if (AActor* ExistingTileset = RealTileset.Get())
	{
		ExistingTileset->Destroy();
		RealTileset.Reset();
	}
	if (AActor* ExistingSunSky = RealSunSky.Get())
	{
		ExistingSunSky->Destroy();
		RealSunSky.Reset();
	}
}

void AMViewGlobeGameMode::SpawnActiveDemo()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (AMViewDemoActor* Existing = ActiveDemo.Get())
	{
		Existing->Destroy();
		ActiveDemo.Reset();
	}
	DestroyGlobeActors();

	if (ActiveKind == EMViewDemoKind::Globe)
	{
		SpawnRealGlobe();
		PlaceGeodeticCamera();
		return;
	}

	UClass* DemoClass = nullptr;
	switch (ActiveKind)
	{
	case EMViewDemoKind::Globe:
		break;
	case EMViewDemoKind::Groundwater:
		DemoClass = AMViewGroundwaterDemo::StaticClass();
		break;
	case EMViewDemoKind::WindField:
		DemoClass = AMViewWindFieldDemo::StaticClass();
		break;
	case EMViewDemoKind::Geology:
		DemoClass = AMViewGeologyDemo::StaticClass();
		break;
	case EMViewDemoKind::RiverFlood:
		DemoClass = AMViewRiverFloodDemo::StaticClass();
		break;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMViewDemoActor* Demo =
		World->SpawnActor<AMViewDemoActor>(DemoClass, FVector::ZeroVector,
										   FRotator::ZeroRotator, Params);
	ActiveDemo = Demo;
	PlaceCameraForDemo(Demo);
}

void AMViewGlobeGameMode::PlaceCameraForDemo(AMViewDemoActor* Demo)
{
	if (!Demo)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	AMViewCameraPawn* Pawn = Cast<AMViewCameraPawn>(PC->GetPawn());
	if (!Pawn)
	{
		Pawn = CameraPawn.Get();
	}
	if (!Pawn)
	{
		return;
	}

	CameraPawn = Pawn;
	Pawn->BeginLocalBrowse(Demo->GetFocusPoint(), Demo->GetSuggestedViewLocation());
	Pawn->RegisterView(FVector2D(1280.0f, 720.0f));

	PC->SetViewTarget(Pawn);
	UE_LOG(LogTemp, Display, TEXT("MView camera framed for %s at %s"),
		   MViewDemoKindName(Demo->GetDemoKind()),
		   *Demo->GetSuggestedViewLocation().ToCompactString());
}

void AMViewGlobeGameMode::PlaceGeodeticCamera()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}
	AMViewCameraPawn* Pawn = Cast<AMViewCameraPawn>(PC->GetPawn());
	if (!Pawn)
	{
		Pawn = CameraPawn.Get();
	}
	if (!Pawn)
	{
		return;
	}
	CameraPawn = Pawn;
	Pawn->BeginGeodeticBrowse();
	Pawn->RegisterView(FVector2D(1280.0f, 720.0f));
	PC->SetViewTarget(Pawn);
}

void AMViewGlobeGameMode::SpawnRealGlobe()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Cesium globe extents far beyond the default UE world box; without this the
	// pawn is yanked back and the viewport stays sky-blue.
	if (AWorldSettings* WorldSettings = World->GetWorldSettings())
	{
		WorldSettings->bEnableWorldBoundsChecks = false;
	}

	if (ACesiumGeoreference* Geo = ACesiumGeoreference::GetDefaultGeoreference(this))
	{
		Geo->SetOriginLongitudeLatitudeHeight(FVector(121.4737, 31.2304, 0.0));
	}

	// Atmosphere + sun for Google Earth-like limb and daylight (A).
	FActorSpawnParameters SunParams;
	SunParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACesiumSunSky* SunSky =
		World->SpawnActor<ACesiumSunSky>(ACesiumSunSky::StaticClass(), FVector::ZeroVector,
										 FRotator::ZeroRotator, SunParams);
	if (SunSky)
	{
		SunSky->TimeZone = 8.0;
		SunSky->SolarTime = 13.0;
		SunSky->UpdateSun();
		if (SunSky->DirectionalLight)
		{
			// Avoid washing out without "Extend Default Luminance Range".
			SunSky->DirectionalLight->SetIntensity(10.0f);
		}
		RealSunSky = SunSky;
		UE_LOG(LogTemp, Display, TEXT("MView atmosphere: CesiumSunSky (CST noon)"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MView failed to spawn CesiumSunSky"));
	}

	const FString Token = FPlatformMisc::GetEnvironmentVariable(TEXT("CESIUM_ION_TOKEN"));
	const FString Url = FPlatformMisc::GetEnvironmentVariable(TEXT("MVIEW_TILESET_URL"));
	const FString AssetText = FPlatformMisc::GetEnvironmentVariable(TEXT("CESIUM_ION_ASSET"));
	// Default ion asset is Google Photorealistic 3D Tiles (B). Override with CESIUM_ION_ASSET.
	const int64 AssetId =
		AssetText.IsEmpty() ? kGooglePhotorealisticAssetId : FCString::Atoi64(*AssetText);
	const bool bPhotorealistic = (AssetId == kGooglePhotorealisticAssetId);

	FTransform SpawnTransform = FTransform::Identity;
	ACesium3DTileset* Tileset = World->SpawnActorDeferred<ACesium3DTileset>(
		ACesium3DTileset::StaticClass(), SpawnTransform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Tileset)
	{
		UE_LOG(LogTemp, Error, TEXT("MView failed to spawn Cesium3DTileset"));
		return;
	}
	Tileset->SetMaximumScreenSpaceError(16.0);
	Tileset->ShowCreditsOnScreen = true;

	if (!Token.IsEmpty())
	{
		Tileset->SetTilesetSource(ETilesetSource::FromCesiumIon);
		Tileset->SetIonAssetID(AssetId);
		Tileset->SetIonAccessToken(Token);
		UE_LOG(LogTemp, Display,
			   TEXT("MView tileset: Cesium ion asset %lld%s"), AssetId,
			   bPhotorealistic ? TEXT(" (Google Photorealistic 3D Tiles)") : TEXT(""));
	}
	else if (!Url.IsEmpty())
	{
		Tileset->SetTilesetSource(ETilesetSource::FromUrl);
		Tileset->SetUrl(Url);
		UE_LOG(LogTemp, Display, TEXT("MView tileset: %s"), *Url);
	}
	else
	{
		Tileset->SetTilesetSource(ETilesetSource::FromEllipsoid);
		UE_LOG(LogTemp, Display,
			   TEXT("MView tileset: ellipsoid + MapLibre demotiles "
					"(CESIUM_ION_TOKEN / MVIEW_IMAGERY_URL / MVIEW_VECTOR_TILES_URL to override)"));
	}
	UGameplayStatics::FinishSpawningActor(Tileset, SpawnTransform);
	RealTileset = Tileset;

	const FString ImageryUrl =
		FPlatformMisc::GetEnvironmentVariable(TEXT("MVIEW_IMAGERY_URL"));
	const FString VectorTilesUrl =
		FPlatformMisc::GetEnvironmentVariable(TEXT("MVIEW_VECTOR_TILES_URL"));

	FString OverlayLabel;
	// Photorealistic meshes already carry imagery — do not stack overlays.
	if (!Token.IsEmpty() && !bPhotorealistic)
	{
		const FString ImageryText =
			FPlatformMisc::GetEnvironmentVariable(TEXT("CESIUM_ION_IMAGERY"));
		const int64 ImageryId =
			ImageryText.IsEmpty() ? kDefaultIonImageryAssetId : FCString::Atoi64(*ImageryText);
		UCesiumIonRasterOverlay* Overlay = NewObject<UCesiumIonRasterOverlay>(Tileset);
		Overlay->IonAssetID = ImageryId;
		Overlay->IonAccessToken = Token;
		Overlay->RegisterComponent();
		Tileset->AddInstanceComponent(Overlay);
		Overlay->AddToTileset();
		OverlayLabel = FString::Printf(TEXT("ion-imagery:%lld"), ImageryId);
	}
	else if (Token.IsEmpty() && !ImageryUrl.IsEmpty())
	{
		// Explicit XYZ/raster template (e.g. CyclOSM) when vector tiles are not wanted.
		UCesiumUrlTemplateRasterOverlay* Overlay =
			NewObject<UCesiumUrlTemplateRasterOverlay>(Tileset);
		Overlay->TemplateUrl = ImageryUrl;
		Overlay->Projection = ECesiumUrlTemplateRasterOverlayProjection::WebMercator;
		Overlay->RegisterComponent();
		Tileset->AddInstanceComponent(Overlay);
		Overlay->AddToTileset();
		OverlayLabel = TEXT("xyz");
		UE_LOG(LogTemp, Display, TEXT("MView imagery template: %s"), *ImageryUrl);
	}
	else if (Token.IsEmpty())
	{
		// Default: MapLibre demotiles (MVT via TileJSON). Reachable when Esri times out.
		const FString TilesJson =
			VectorTilesUrl.IsEmpty() ? FString(kDefaultMapLibreTilesJson) : VectorTilesUrl;
		UCesiumVectorTilesRasterOverlay* Overlay =
			NewObject<UCesiumVectorTilesRasterOverlay>(Tileset);
		Overlay->Source = ECesiumVectorTilesRasterOverlaySource::FromUrl;
		Overlay->Url = TilesJson;
		Overlay->DefaultStyle.PolygonStyle.Fill = true;
		Overlay->DefaultStyle.PolygonStyle.FillStyle.Color = FColor(0x4a, 0x90, 0x5a, 0xff);
		Overlay->DefaultStyle.PolygonStyle.Outline = true;
		Overlay->DefaultStyle.PolygonStyle.OutlineStyle.Color = FColor(0x1e, 0x3a, 0x28, 0xff);
		Overlay->DefaultStyle.PolygonStyle.OutlineStyle.Width = 1.0;
		Overlay->DefaultStyle.LineStyle.Color = FColor(0x2c, 0x5a, 0x8a, 0xff);
		Overlay->DefaultStyle.LineStyle.Width = 1.5;
		Overlay->RegisterComponent();
		Tileset->AddInstanceComponent(Overlay);
		Overlay->AddToTileset();
		OverlayLabel = TEXT("maplibre-mvt");
		UE_LOG(LogTemp, Display, TEXT("MView MapLibre vector tiles: %s"), *TilesJson);
	}

	const FString MarkerPath = FPaths::ProjectSavedDir() / TEXT("mview_demo_ready.txt");
	FString TilesetLabel;
	if (!Token.IsEmpty())
	{
		TilesetLabel = FString::Printf(TEXT("ion:%lld"), AssetId);
	}
	else if (!Url.IsEmpty())
	{
		TilesetLabel = Url;
	}
	else if (!OverlayLabel.IsEmpty())
	{
		TilesetLabel = FString::Printf(TEXT("ellipsoid+%s"), *OverlayLabel);
	}
	else
	{
		TilesetLabel = TEXT("ellipsoid");
	}
	const FString Marker = FString::Printf(
		TEXT("globe tileset=%s photorealistic=%d sunsky=%d\n"), *TilesetLabel,
		bPhotorealistic && !Token.IsEmpty() ? 1 : 0, RealSunSky.IsValid() ? 1 : 0);
	FFileHelper::SaveStringToFile(Marker, *MarkerPath);
}
