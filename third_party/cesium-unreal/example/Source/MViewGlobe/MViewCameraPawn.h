// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MViewCameraPawn.generated.h"

class UCameraComponent;
class UCesiumFlyToComponent;
class UCesiumGlobeAnchorComponent;

// Globe-aware camera pawn. Registers into ACesiumCameraManager::AdditionalCameras.
// Field demos use a short local flight; the Google Earth-style globe view starts
// in space and FlyTo's into the city.
UCLASS()
class MVIEWGLOBE_API AMViewCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AMViewCameraPawn();

	UCameraComponent* GetViewCamera() const { return ViewCamera; }

	// Writes this pawn into AdditionalCameras. AddCamera is obsolete.
	void RegisterView(const FVector2D& ViewportSize);

	// Local orbit around a procedural demo, then a short flight to SuggestedEye.
	void BeginLocalBrowse(const FVector& Focus, const FVector& SuggestedEye);

	// Places the pawn above Earth and starts UCesiumFlyToComponent into Shanghai.
	void BeginGeodeticBrowse();

protected:
	virtual void Tick(float DeltaSeconds) override;

private:
	void ApplyLookAt(const FVector& Eye, const FVector& Target);

	UPROPERTY(VisibleAnywhere)
	UCesiumGlobeAnchorComponent* GlobeAnchor;

	UPROPERTY(VisibleAnywhere)
	UCesiumFlyToComponent* FlyTo;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	bool bLocalFlight = false;
	float LocalFlightElapsed = 0.0f;
	float LocalFlightDuration = 2.8f;
	FVector LocalFlightStart = FVector::ZeroVector;
	FVector LocalFlightEnd = FVector::ZeroVector;
	FVector LocalFocus = FVector::ZeroVector;
};
