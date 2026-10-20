// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MViewGlobeTarget : TargetRules
{
	public MViewGlobeTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("MViewGlobe");
	}
}
