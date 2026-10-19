// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

using UnrealBuildTool;

public class MViewGlobe : ModuleRules
{
	public MViewGlobe(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"CesiumRuntime",
		});
	}
}
