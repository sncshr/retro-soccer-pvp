// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PocketStriker : ModuleRules
{
	public PocketStriker(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] 
		{
			"AnimGraphRuntime"
		});

		// Module organization
		PublicIncludePaths.AddRange(new string[]
		{
			"PocketStriker/Gameplay",
			"PocketStriker/Animation",
			"PocketStriker/AI",
			"PocketStriker/Network",
			"PocketStriker/Tools"
		});
	}
}
