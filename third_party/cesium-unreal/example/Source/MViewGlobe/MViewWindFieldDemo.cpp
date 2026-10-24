// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "MViewWindFieldDemo.h"

AMViewWindFieldDemo::AMViewWindFieldDemo()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMViewWindFieldDemo::Tick(float DeltaSeconds)
{
	PhaseSeconds += DeltaSeconds;
	Rebuild();
	Super::Tick(DeltaSeconds);
}

void AMViewWindFieldDemo::BuildDemo(FMViewDemoMeshBuilder& OutBuilder)
{
	const FColor Ground(48, 56, 48);
	const FColor Gust(90, 180, 255);
	const FColor Core(255, 220, 90);

	OutBuilder.AddBox(FVector(0.0f, 0.0f, -2.0f), FVector(220.0f, 160.0f, 0.0f), Ground);

	constexpr int32 Columns = 14;
	constexpr int32 Rows = 10;
	constexpr float Cell = 14.0f;
	for (int32 Row = 0; Row < Rows; ++Row)
	{
		for (int32 Column = 0; Column < Columns; ++Column)
		{
			const float X = 20.0f + Column * Cell;
			const float Y = 20.0f + Row * Cell;
			const float Z = 18.0f + 8.0f * FMath::Sin(0.08f * X + 0.05f * Y);
			const float U = 18.0f + 10.0f * FMath::Sin(0.04f * Y + PhaseSeconds);
			const float V = 6.0f * FMath::Cos(0.05f * X - PhaseSeconds * 0.7f);
			const FVector Start(X, Y, Z);
			const FVector End = Start + FVector(U, V, 1.5f);
			const bool bFast = FVector2D(U, V).Size() > 22.0f;
			OutBuilder.AddSegment(Start, End, bFast ? Core : Gust, bFast ? 3.5f : 2.0f);
			OutBuilder.AddBoxCentered(End, FVector(2.2f, 2.2f, 2.2f), bFast ? Core : Gust);
		}
	}
}
