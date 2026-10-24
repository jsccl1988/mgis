// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "MViewDemoActor.h"
#include "MViewWindFieldDemo.generated.h"

// Synthetic U/V wind field drawn as advecting streak lines (fake data).
UCLASS()
class MVIEWGLOBE_API AMViewWindFieldDemo : public AMViewDemoActor
{
	GENERATED_BODY()

public:
	AMViewWindFieldDemo();

	virtual EMViewDemoKind GetDemoKind() const override { return EMViewDemoKind::WindField; }

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BuildDemo(FMViewDemoMeshBuilder& OutBuilder) override;

private:
	float PhaseSeconds = 0.0f;
};
