// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "MViewCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "CesiumCamera.h"
#include "CesiumCameraManager.h"
#include "CesiumGlobeAnchorComponent.h"

AMViewCameraPawn::AMViewCameraPawn()
{
	PrimaryActorTick.bCanEverTick = false;
	GlobeAnchor = CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("GlobeAnchor"));
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	RootComponent = GlobeAnchor;
	ViewCamera->SetupAttachment(GlobeAnchor);
}

void AMViewCameraPawn::RegisterView(const FVector2D& ViewportSize)
{
	ACesiumCameraManager* Manager = ACesiumCameraManager::GetDefaultCameraManager(this);
	if (!Manager)
	{
		return;
	}
	Manager->UsePlayerCameras = false;
	Manager->UseEditorCameras = false;

	FCesiumCamera Camera;
	Camera.ViewportSize = ViewportSize;
	Camera.CameraParameterSource = ECameraParameterSource::CameraComponent;
	Camera.CameraComponent = ViewCamera;
	Manager->AdditionalCameras = {Camera};
}
