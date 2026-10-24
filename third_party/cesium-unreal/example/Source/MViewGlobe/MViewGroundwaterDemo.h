// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "MViewDemoActor.h"
#include "MViewGroundwaterDemo.generated.h"

// Cutaway aquifer stack: bedrock, confined, aquitard, water table, wells.
UCLASS()
class MVIEWGLOBE_API AMViewGroundwaterDemo : public AMViewDemoActor
{
	GENERATED_BODY()

public:
	virtual EMViewDemoKind GetDemoKind() const override { return EMViewDemoKind::Groundwater; }

protected:
	virtual void BuildDemo(FMViewDemoMeshBuilder& OutBuilder) override;
};
