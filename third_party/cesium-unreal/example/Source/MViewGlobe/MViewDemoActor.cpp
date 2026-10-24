// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "MViewDemoActor.h"

#include "DrawDebugHelpers.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

AMViewDemoActor::AMViewDemoActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AMViewDemoActor::BeginPlay()
{
	Super::BeginPlay();
	Rebuild();
	UE_LOG(LogTemp, Display, TEXT("MView demo ready: %s boxes=%d segments=%d"),
		   MViewDemoKindName(GetDemoKind()), GetBoxCount(), GetSegmentCount());

	const FString MarkerPath =
		FPaths::ProjectSavedDir() / TEXT("mview_demo_ready.txt");
	const FString Marker = FString::Printf(
		TEXT("%s boxes=%d segments=%d\n"), MViewDemoKindName(GetDemoKind()),
		GetBoxCount(), GetSegmentCount());
	FFileHelper::SaveStringToFile(Marker, *MarkerPath);
}

void AMViewDemoActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	DrawDemo();
}

void AMViewDemoActor::Rebuild()
{
	Builder.Reset();
	BuildDemo(Builder);
	FocusPoint = Builder.ComputeFocusPoint();
	FrameSuggestedCamera();
}

void AMViewDemoActor::FrameSuggestedCamera()
{
	const FBox Bounds = Builder.ComputeBounds();
	if (!Bounds.IsValid)
	{
		return;
	}
	const FVector Center = Bounds.GetCenter();
	const float Radius = Bounds.GetExtent().Size();
	const float Distance = FMath::Max(220.0f, Radius * 2.2f);
	SuggestedViewLocation = Center + FVector(-Distance * 0.65f, -Distance * 0.95f, Distance * 0.72f);
	SuggestedViewRotation = (Center - SuggestedViewLocation).Rotation();
}

void AMViewDemoActor::DrawDemo() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FTransform& Xform = GetActorTransform();
	for (const FMViewDemoBox& Box : Builder.GetBoxes())
	{
		DrawDebugSolidBox(World, Xform.TransformPosition(Box.Center), Box.Extent,
						  Xform.GetRotation(), Box.Color, false, -1.0f, SDPG_World);
	}
	for (const FMViewDemoSegment& Segment : Builder.GetSegments())
	{
		DrawDebugLine(World, Xform.TransformPosition(Segment.Start),
					  Xform.TransformPosition(Segment.End), Segment.Color, false, -1.0f,
					  SDPG_World, Segment.Thickness);
	}
}
