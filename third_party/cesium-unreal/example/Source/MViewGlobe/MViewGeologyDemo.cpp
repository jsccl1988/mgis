// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "MViewGeologyDemo.h"

void AMViewGeologyDemo::BuildDemo(FMViewDemoMeshBuilder& OutBuilder)
{
	const FColor StrataColors[] = {
		FColor(92, 64, 51),
		FColor(160, 120, 72),
		FColor(120, 140, 90),
		FColor(180, 150, 110),
		FColor(70, 90, 110),
		FColor(140, 100, 80),
	};
	constexpr int32 LayerCount = UE_ARRAY_COUNT(StrataColors);
	constexpr int32 Columns = 16;
	constexpr int32 Rows = 10;
	constexpr float Cell = 12.0f;
	constexpr float FaultX = 96.0f;

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
			const float Fold = 10.0f * FMath::Sin(MidX * 0.045f) * FMath::Cos(MidY * 0.03f);
			const float FaultLift = MidX >= FaultX ? 18.0f : 0.0f;
			float Z = Fold + FaultLift;

			for (int32 Layer = 0; Layer < LayerCount; ++Layer)
			{
				const float Thickness = 9.0f + static_cast<float>((Layer + Column) % 3);
				OutBuilder.AddBox(FVector(X0, Y0, Z), FVector(X1, Y1, Z + Thickness),
								  StrataColors[Layer]);
				Z += Thickness;
			}
		}
	}

	// Fault plane marker.
	OutBuilder.AddBox(FVector(FaultX - 1.0f, 0.0f, -5.0f),
					  FVector(FaultX + 1.0f, Rows * Cell, 90.0f), FColor(220, 60, 60));
}
