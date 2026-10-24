// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"

// Selectable field demos hosted by MViewGlobeGameMode.
enum class EMViewDemoKind : uint8
{
	Groundwater,
	WindField,
	Geology,
	RiverFlood,
	Globe,
};

inline const TCHAR* MViewDemoKindName(EMViewDemoKind Kind)
{
	switch (Kind)
	{
	case EMViewDemoKind::Groundwater:
		return TEXT("groundwater");
	case EMViewDemoKind::WindField:
		return TEXT("wind");
	case EMViewDemoKind::Geology:
		return TEXT("geology");
	case EMViewDemoKind::RiverFlood:
		return TEXT("flood");
	case EMViewDemoKind::Globe:
		return TEXT("globe");
	default:
		return TEXT("unknown");
	}
}

inline bool MViewTryParseDemoKind(const FString& Token, EMViewDemoKind& OutKind)
{
	const FString Lower = Token.ToLower();
	if (Lower == TEXT("groundwater") || Lower == TEXT("gw") || Lower == TEXT("water"))
	{
		OutKind = EMViewDemoKind::Groundwater;
		return true;
	}
	if (Lower == TEXT("wind") || Lower == TEXT("windfield") || Lower == TEXT("wind_field"))
	{
		OutKind = EMViewDemoKind::WindField;
		return true;
	}
	if (Lower == TEXT("geology") || Lower == TEXT("geo") || Lower == TEXT("strata"))
	{
		OutKind = EMViewDemoKind::Geology;
		return true;
	}
	if (Lower == TEXT("flood") || Lower == TEXT("river") || Lower == TEXT("riverflood") ||
		Lower == TEXT("river_flood") || Lower == TEXT("inundation"))
	{
		OutKind = EMViewDemoKind::RiverFlood;
		return true;
	}
	if (Lower == TEXT("globe") || Lower == TEXT("imagery") || Lower == TEXT("shanghai") ||
		Lower == TEXT("terrain") || Lower == TEXT("earth") || Lower == TEXT("google") ||
		Lower == TEXT("photorealistic") || Lower == TEXT("photo"))
	{
		OutKind = EMViewDemoKind::Globe;
		return true;
	}
	return false;
}
