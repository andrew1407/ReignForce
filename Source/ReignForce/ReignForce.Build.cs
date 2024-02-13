// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ReignForce : ModuleRules
{
	public ReignForce(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"SlateCore",
			"NNE",					// Neural network engine
			"Json",					// JSON parsing 
			"JsonUtilities",
			"WebSockets",			// ws client connection handling
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "AIModule", "NavigationSystem" });
	}
}
