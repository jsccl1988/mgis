// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "MViewCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "CesiumCamera.h"
#include "CesiumCameraManager.h"
#include "CesiumGlobeAnchorComponent.h"
#include "Components/SceneComponent.h"

AMViewCameraPawn::AMViewCameraPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	// GlobeAnchor is a UActorComponent (not a scene component); root is a plain scene node.
	USceneComponent* SceneRoot =
		CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	GlobeAnchor =
		CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("GlobeAnchor"));
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SceneRoot);
}

void AMViewCameraPawn::RegisterView(const FVector2D& ViewportSize)
{
	ACesiumCameraManager* Manager =
		ACesiumCameraManager::GetDefaultCameraManager(this);
	if (!Manager)
	{
		return;
	}
	Manager->UsePlayerCameras = false;
	Manager->UseEditorCameras = false;

	FCesiumCamera Camera;
	Camera.ViewportSize = ViewportSize;
	Camera.ParameterSource = ECameraParameterSource::CameraComponent;
	Camera.CameraComponent = ViewCamera;
	Manager->AdditionalCameras = {Camera};
}
