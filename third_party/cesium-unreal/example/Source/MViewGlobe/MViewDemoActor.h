// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MViewDemoKinds.h"
#include "MViewDemoMeshBuilder.h"
#include "MViewDemoActor.generated.h"

// Base for procedural field demos. Subclasses fill the builder; this actor
// redraws colored debug solids every tick (no Content materials required).
UCLASS(Abstract)
class MVIEWGLOBE_API AMViewDemoActor : public AActor
{
	GENERATED_BODY()

public:
	AMViewDemoActor();

	virtual EMViewDemoKind GetDemoKind() const PURE_VIRTUAL(AMViewDemoActor::GetDemoKind, return EMViewDemoKind::Groundwater;);

	FVector GetFocusPoint() const { return FocusPoint; }
	FVector GetSuggestedViewLocation() const { return SuggestedViewLocation; }
	FRotator GetSuggestedViewRotation() const { return SuggestedViewRotation; }

	int32 GetBoxCount() const { return Builder.GetBoxes().Num(); }
	int32 GetSegmentCount() const { return Builder.GetSegments().Num(); }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void BuildDemo(FMViewDemoMeshBuilder& OutBuilder) PURE_VIRTUAL(AMViewDemoActor::BuildDemo, );

	void Rebuild();
	void DrawDemo() const;
	void FrameSuggestedCamera();

	FMViewDemoMeshBuilder Builder;
	FVector FocusPoint = FVector::ZeroVector;
	FVector SuggestedViewLocation = FVector(-180.0f, -280.0f, 220.0f);
	FRotator SuggestedViewRotation = FRotator(-25.0f, 35.0f, 0.0f);
};
