// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "MViewDemoActor.h"
#include "MViewRiverFloodDemo.generated.h"

// River channel with animated inundation overbank (fake DEM + stage).
UCLASS()
class MVIEWGLOBE_API AMViewRiverFloodDemo : public AMViewDemoActor
{
	GENERATED_BODY()

public:
	AMViewRiverFloodDemo();

	virtual EMViewDemoKind GetDemoKind() const override { return EMViewDemoKind::RiverFlood; }

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BuildDemo(FMViewDemoMeshBuilder& OutBuilder) override;

private:
	float PhaseSeconds = 0.0f;
};
