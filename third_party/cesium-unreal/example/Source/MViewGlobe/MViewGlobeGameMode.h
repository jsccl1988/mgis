// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MViewDemoKinds.h"
#include "MViewGlobeGameMode.generated.h"

class AMViewCameraPawn;
class AMViewDemoActor;

// Spawns the globe camera pawn and one view.
// Default globe mode: CesiumSunSky atmosphere + space-to-city FlyTo.
// With CESIUM_ION_TOKEN: Google Photorealistic 3D Tiles (ion 2275207) by default.
// Without token: ellipsoid + Esri World Imagery (atmosphere + FlyTo still run).
// Select a procedural field with -MViewDemo=groundwater|wind|geology|flood.
// CESIUM_ION_ASSET overrides the ion tileset id. MVIEW_TILESET_URL loads a URL.
// No token is stored in git.
UCLASS()
class MVIEWGLOBE_API AMViewGlobeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMViewGlobeGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options,
						  FString& ErrorMessage) override;
	virtual void StartPlay() override;

	UFUNCTION(Exec)
	void Demo(const FString& Name);

	EMViewDemoKind GetActiveDemoKind() const { return ActiveKind; }

private:
	void ResolveDemoKind(const FString& Options);
	void SpawnActiveDemo();
	void SpawnRealGlobe();
	void DestroyGlobeActors();
	void PlaceCameraForDemo(AMViewDemoActor* Demo);
	void PlaceGeodeticCamera();

	EMViewDemoKind ActiveKind = EMViewDemoKind::Globe;
	TWeakObjectPtr<AMViewDemoActor> ActiveDemo;
	TWeakObjectPtr<AMViewCameraPawn> CameraPawn;
	TWeakObjectPtr<AActor> RealTileset;
	TWeakObjectPtr<AActor> RealSunSky;
};
