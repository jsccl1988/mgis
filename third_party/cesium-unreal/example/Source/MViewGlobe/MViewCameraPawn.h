// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MViewCameraPawn.generated.h"

class UCameraComponent;
class UCesiumGlobeAnchorComponent;

// One globe camera. The product registers it on ACesiumCameraManager::AdditionalCameras.
UCLASS()
class MVIEWGLOBE_API AMViewCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AMViewCameraPawn();

	UCameraComponent* GetViewCamera() const { return ViewCamera; }

	// Writes this pawn into AdditionalCameras. AddCamera is obsolete.
	void RegisterView(const FVector2D& ViewportSize);

private:
	UPROPERTY(VisibleAnywhere)
	UCesiumGlobeAnchorComponent* GlobeAnchor;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;
};
