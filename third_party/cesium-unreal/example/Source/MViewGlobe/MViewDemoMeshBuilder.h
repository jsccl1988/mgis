// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"

struct FMViewDemoBox
{
	FVector Center = FVector::ZeroVector;
	FVector Extent = FVector::ZeroVector;
	FColor Color = FColor::White;
};

struct FMViewDemoSegment
{
	FVector Start = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	FColor Color = FColor::White;
	float Thickness = 2.0f;
};

// Accumulates axis-aligned boxes and line segments for debug draw + optional mesh.
class FMViewDemoMeshBuilder
{
public:
	void Reset();

	void AddBox(const FVector& Min, const FVector& Max, const FColor& Color);
	void AddBoxCentered(const FVector& Center, const FVector& Size, const FColor& Color);
	void AddSegment(const FVector& Start, const FVector& End, const FColor& Color,
					float Thickness = 2.0f);

	const TArray<FMViewDemoBox>& GetBoxes() const { return Boxes; }
	const TArray<FMViewDemoSegment>& GetSegments() const { return Segments; }

	FBox ComputeBounds() const;
	FVector ComputeFocusPoint() const;

private:
	TArray<FMViewDemoBox> Boxes;
	TArray<FMViewDemoSegment> Segments;
};
