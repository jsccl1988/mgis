// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "MViewRiverFloodDemo.h"

AMViewRiverFloodDemo::AMViewRiverFloodDemo()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMViewRiverFloodDemo::Tick(float DeltaSeconds)
{
	PhaseSeconds += DeltaSeconds;
	Rebuild();
	Super::Tick(DeltaSeconds);
}

void AMViewRiverFloodDemo::BuildDemo(FMViewDemoMeshBuilder& OutBuilder)
{
	const FColor Bank(110, 130, 80);
	const FColor Channel(40, 70, 50);
	const FColor Water(30, 110, 190);
	const FColor Flood(50, 150, 220);

	constexpr int32 Columns = 24;
	constexpr int32 Rows = 14;
	constexpr float Cell = 10.0f;

	const float Stage =
		12.0f + 6.0f * (0.5f + 0.5f * FMath::Sin(PhaseSeconds * 0.55f));

	for (int32 Row = 0; Row < Rows; ++Row)
	{
		for (int32 Column = 0; Column < Columns; ++Column)
		{
			const float X0 = Column * Cell;
			const float Y0 = Row * Cell;
			const float X1 = X0 + Cell;
			const float Y1 = Y0 + Cell;
			const float MidX = (X0 + X1) * 0.5f;
			const float MidY = (Y0 + Y1) * 0.5f;

			const float RiverY = 70.0f + 18.0f * FMath::Sin(MidX * 0.035f);
			const float Dist = FMath::Abs(MidY - RiverY);
			float GroundZ = 16.0f + 3.0f * FMath::Sin(MidX * 0.02f) + 2.0f * FMath::Cos(MidY * 0.04f);
			if (Dist < 12.0f)
			{
				GroundZ = 6.0f + Dist * 0.35f;
				OutBuilder.AddBox(FVector(X0, Y0, 0.0f), FVector(X1, Y1, GroundZ), Channel);
			}
			else
			{
				OutBuilder.AddBox(FVector(X0, Y0, 0.0f), FVector(X1, Y1, GroundZ), Bank);
			}

			if (Stage > GroundZ)
			{
				const FColor Fill = Dist < 14.0f ? Water : Flood;
				OutBuilder.AddBox(FVector(X0, Y0, GroundZ), FVector(X1, Y1, Stage), Fill);
			}
		}
	}

	// Levee remnants that get overtopped at high stage.
	OutBuilder.AddBox(FVector(40.0f, 40.0f, 0.0f), FVector(200.0f, 44.0f, 14.5f),
					  FColor(150, 140, 120));
	OutBuilder.AddBox(FVector(40.0f, 96.0f, 0.0f), FVector(200.0f, 100.0f, 14.5f),
					  FColor(150, 140, 120));
}
