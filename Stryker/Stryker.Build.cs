// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Stryker : ModuleRules
{
	public Stryker(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" ,"Niagara" ,"Slate" ,"SlateCore"});
    }
}
