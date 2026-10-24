// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "MViewDemoMeshBuilder.h"

void FMViewDemoMeshBuilder::Reset()
{
	Boxes.Reset();
	Segments.Reset();
}

void FMViewDemoMeshBuilder::AddBox(const FVector& Min, const FVector& Max, const FColor& Color)
{
	const FVector SafeMin(FMath::Min(Min.X, Max.X), FMath::Min(Min.Y, Max.Y),
						  FMath::Min(Min.Z, Max.Z));
	const FVector SafeMax(FMath::Max(Min.X, Max.X), FMath::Max(Min.Y, Max.Y),
						  FMath::Max(Min.Z, Max.Z));
	FMViewDemoBox Box;
	Box.Center = (SafeMin + SafeMax) * 0.5f;
	Box.Extent = (SafeMax - SafeMin) * 0.5f;
	Box.Color = Color;
	if (Box.Extent.GetMin() > KINDA_SMALL_NUMBER)
	{
		Boxes.Add(Box);
	}
}

void FMViewDemoMeshBuilder::AddBoxCentered(const FVector& Center, const FVector& Size,
										   const FColor& Color)
{
	AddBox(Center - Size * 0.5f, Center + Size * 0.5f, Color);
}

void FMViewDemoMeshBuilder::AddSegment(const FVector& Start, const FVector& End,
									   const FColor& Color, float Thickness)
{
	FMViewDemoSegment Segment;
	Segment.Start = Start;
	Segment.End = End;
	Segment.Color = Color;
	Segment.Thickness = Thickness;
	Segments.Add(Segment);
}

FBox FMViewDemoMeshBuilder::ComputeBounds() const
{
	FBox Bounds(ForceInit);
	for (const FMViewDemoBox& Box : Boxes)
	{
		Bounds += Box.Center - Box.Extent;
		Bounds += Box.Center + Box.Extent;
	}
	for (const FMViewDemoSegment& Segment : Segments)
	{
		Bounds += Segment.Start;
		Bounds += Segment.End;
	}
	return Bounds;
}

FVector FMViewDemoMeshBuilder::ComputeFocusPoint() const
{
	const FBox Bounds = ComputeBounds();
	return Bounds.IsValid ? Bounds.GetCenter() : FVector::ZeroVector;
}
