// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "MViewDemoActor.h"
#include "MViewGeologyDemo.generated.h"

// Folded strata with a vertical fault offset (synthetic 3D geology).
UCLASS()
class MVIEWGLOBE_API AMViewGeologyDemo : public AMViewDemoActor
{
	GENERATED_BODY()

public:
	virtual EMViewDemoKind GetDemoKind() const override { return EMViewDemoKind::Geology; }

protected:
	virtual void BuildDemo(FMViewDemoMeshBuilder& OutBuilder) override;
};
