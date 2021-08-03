// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProneSystem : ModuleRules
{
	public ProneSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
