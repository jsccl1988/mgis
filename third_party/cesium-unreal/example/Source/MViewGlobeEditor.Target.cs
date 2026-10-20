// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MViewGlobeEditorTarget : TargetRules
{
	public MViewGlobeEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("MViewGlobe");
	}
}
