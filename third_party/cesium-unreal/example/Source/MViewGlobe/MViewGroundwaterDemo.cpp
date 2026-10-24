// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "MViewGroundwaterDemo.h"

namespace
{
float WaterTable(float X, float Y)
{
	return 74.0f + 7.0f * FMath::Sin(X * 0.035f) + 3.5f * FMath::Cos(Y * 0.05f);
}

void AddWell(FMViewDemoMeshBuilder& Builder, float X, float Y, float Table)
{
	const FColor Well(210, 214, 219);
	const FColor Column(51, 158, 230);
	Builder.AddBox(FVector(X, Y, 0.0f), FVector(X + 5.0f, Y + 5.0f, 112.0f), Well);
	Builder.AddBox(FVector(X + 1.4f, Y + 1.4f, 20.0f),
				   FVector(X + 3.6f, Y + 3.6f, Table), Column);
	Builder.AddBox(FVector(X - 2.0f, Y - 2.0f, 112.0f),
				   FVector(X + 7.0f, Y + 7.0f, 115.0f), Well);
}
}  // namespace

void AMViewGroundwaterDemo::BuildDemo(FMViewDemoMeshBuilder& OutBuilder)
{
	const FColor Bedrock(107, 92, 82);
	const FColor Confined(31, 82, 148);
	const FColor Aquitard(102, 117, 66);
	const FColor Saturated(41, 133, 199);
	const FColor DrySand(199, 173, 107);
	const FColor Soil(97, 66, 41);
	const FColor Pond(26, 97, 158);

	constexpr int32 Columns = 12;
	constexpr int32 Rows = 8;
	constexpr float Cell = 18.0f;

	for (int32 Row = 0; Row < Rows; ++Row)
	{
		for (int32 Column = 0; Column < Columns; ++Column)
		{
			if (Column > 8 && Row > 5)
			{
				continue;
			}
			const float X = static_cast<float>(Column) * Cell;
			const float Y = static_cast<float>(Row) * Cell;
			const float Table = WaterTable(X + Cell * 0.5f, Y + Cell * 0.5f);
			OutBuilder.AddBox(FVector(X, Y, 0.0f), FVector(X + Cell, Y + Cell, 22.0f), Bedrock);
			OutBuilder.AddBox(FVector(X, Y, 22.0f), FVector(X + Cell, Y + Cell, 48.0f), Confined);
			OutBuilder.AddBox(FVector(X, Y, 48.0f), FVector(X + Cell, Y + Cell, 62.0f), Aquitard);
			OutBuilder.AddBox(FVector(X, Y, 62.0f), FVector(X + Cell, Y + Cell, Table), Saturated);
			OutBuilder.AddBox(FVector(X, Y, Table), FVector(X + Cell, Y + Cell, 98.0f), DrySand);
			OutBuilder.AddBox(FVector(X, Y, 98.0f), FVector(X + Cell, Y + Cell, 108.0f), Soil);
		}
	}

	OutBuilder.AddBox(FVector(36.0f, 36.0f, 108.2f), FVector(72.0f, 64.0f, 110.6f), Pond);
	AddWell(OutBuilder, 90.0f, 54.0f, WaterTable(92.0f, 56.0f));
	AddWell(OutBuilder, 150.0f, 36.0f, WaterTable(152.0f, 38.0f));
}
