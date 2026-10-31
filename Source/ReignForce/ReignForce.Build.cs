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
			"UMG",
			"Slate",
			"CommonUI",				// Menus: activatable widgets, input routing (back & click actions)
			"CommonInput",
			"DeveloperSettings",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "AIModule", "NavigationSystem", "MoviePlayer", "PhysicsCore" });

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");	// Automation tests PIE session control
		}
	}
}
