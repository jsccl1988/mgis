// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "MViewCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "CesiumCamera.h"
#include "CesiumCameraManager.h"
#include "CesiumFlyToComponent.h"
#include "CesiumGlobeAnchorComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"

namespace
{
// Shanghai Bund — default Google Earth-style destination.
constexpr double kShanghaiLon = 121.4737;
constexpr double kShanghaiLat = 31.2304;
// Full-disk Earth start height (meters above ellipsoid).
constexpr double kSpaceHeightMeters = 1.2e7;
// City-scale eye height after FlyTo.
constexpr double kCityHeightMeters = 850.0;
}  // namespace

AMViewCameraPawn::AMViewCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// GlobeAnchor is a UActorComponent (not a scene component); root is a plain scene node.
	USceneComponent* SceneRoot =
		CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	GlobeAnchor =
		CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("GlobeAnchor"));
	FlyTo = CreateDefaultSubobject<UCesiumFlyToComponent>(TEXT("FlyTo"));
	FlyTo->Duration = 8.0f;
	FlyTo->RotationToUse = ECesiumFlyToRotation::Actor;
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SceneRoot);
	// GlobeAnchor keeps actor Z as geodetic up; camera relative pitch looks at Earth.
	ViewCamera->bUsePawnControlRotation = false;
	ViewCamera->FieldOfView = 70.0f;
}

void AMViewCameraPawn::RegisterView(const FVector2D& ViewportSize)
{
	ACesiumCameraManager* Manager =
		ACesiumCameraManager::GetDefaultCameraManager(this);
	if (!Manager)
	{
		return;
	}
	// Keep player cameras so Cesium sees the possessed view even if AdditionalCameras lags.
	Manager->UsePlayerCameras = true;
	Manager->UseEditorCameras = false;

	FCesiumCamera Camera;
	Camera.ViewportSize = ViewportSize;
	Camera.ParameterSource = ECameraParameterSource::CameraComponent;
	Camera.CameraComponent = ViewCamera;
	Manager->AdditionalCameras = {Camera};
}

void AMViewCameraPawn::BeginLocalBrowse(const FVector& Focus, const FVector& SuggestedEye)
{
	bLocalFlight = true;
	LocalFlightElapsed = 0.0f;
	LocalFlightDuration = 2.8f;
	LocalFocus = Focus;
	LocalFlightEnd = SuggestedEye;

	const FVector Offset = SuggestedEye - Focus;
	const float Radius = FMath::Max(180.0f, Offset.Size() * 0.85f);
	LocalFlightStart =
		Focus + FVector(-Radius * 0.9f, Radius * 0.35f, Radius * 0.55f);
	ApplyLookAt(LocalFlightStart, Focus);

	UE_LOG(LogTemp, Display, TEXT("MView local browse: %s -> %s"),
		   *LocalFlightStart.ToCompactString(), *LocalFlightEnd.ToCompactString());
}

void AMViewCameraPawn::BeginGeodeticBrowse()
{
	bLocalFlight = false;

	if (GlobeAnchor)
	{
		GlobeAnchor->MoveToLongitudeLatitudeHeight(
			FVector(kShanghaiLon, kShanghaiLat, kSpaceHeightMeters));
		const FVector Llh = GlobeAnchor->GetLongitudeLatitudeHeight();
		UE_LOG(LogTemp, Display,
			   TEXT("MView globe anchor after space place: lon=%.4f lat=%.4f h=%.0f unreal=%s"),
			   Llh.X, Llh.Y, Llh.Z, *GetActorLocation().ToCompactString());
	}

	// Actor Z = geodetic up after MoveTo; pitch -90 looks along -Z toward the ellipsoid.
	if (ViewCamera)
	{
		ViewCamera->bUsePawnControlRotation = false;
		ViewCamera->SetRelativeRotation(FRotator(-90.0, 0.0, 0.0));
	}
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetControlRotation(FRotator(-90.0, 0.0, 0.0));
	}

	if (FlyTo)
	{
		FlyTo->Duration = 8.0f;
		FlyTo->RotationToUse = ECesiumFlyToRotation::Actor;
		FlyTo->FlyToLocationLongitudeLatitudeHeight(
			FVector(kShanghaiLon, kShanghaiLat, kCityHeightMeters),
			/*YawAtDestination=*/35.0,
			/*PitchAtDestination=*/-28.0,
			/*CanInterruptByMoving=*/false);
	}

	UE_LOG(LogTemp, Display,
		   TEXT("MView Google Earth browse: space %.0f m -> Shanghai lon %.3f lat %.3f h %.0f"),
		   kSpaceHeightMeters, kShanghaiLon, kShanghaiLat, kCityHeightMeters);
}

void AMViewCameraPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bLocalFlight)
	{
		return;
	}

	LocalFlightElapsed += DeltaSeconds;
	const float Alpha =
		FMath::Clamp(LocalFlightElapsed / LocalFlightDuration, 0.0f, 1.0f);
	const float Smooth = Alpha * Alpha * (3.0f - 2.0f * Alpha);
	const FVector Eye = FMath::Lerp(LocalFlightStart, LocalFlightEnd, Smooth);
	ApplyLookAt(Eye, LocalFocus);

	if (Alpha >= 1.0f)
	{
		bLocalFlight = false;
	}
}

void AMViewCameraPawn::ApplyLookAt(const FVector& Eye, const FVector& Target)
{
	SetActorLocation(Eye);
	const FRotator Look = (Target - Eye).Rotation();
	SetActorRotation(Look);
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetControlRotation(Look);
	}
	if (ViewCamera)
	{
		ViewCamera->SetWorldLocation(Eye);
		ViewCamera->SetWorldRotation(Look);
	}
}
