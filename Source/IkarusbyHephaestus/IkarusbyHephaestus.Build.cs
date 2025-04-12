// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class IkarusbyHephaestus : ModuleRules
{
	public IkarusbyHephaestus(ReadOnlyTargetRules Target) : base(Target)
	{
		// Use explicit or shared precompiled headers to optimize compile times.
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public dependencies are modules that are used in your public API.
		// They include core engine and gameplay modules.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			// AI and gameplay functionality
			"AIModule",
			"GameplayTasks",
			"NavigationSystem",
			// UI Modules - useful if you plan to implement in-game debugging tools or UIs.
			"UMG",
			"Slate",
			"SlateCore",
			// JSON processing for configuration and data handling
			"Json",
			"JsonUtilities"
		});

		// Private dependencies are used internally by this module.
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Online and multiplayer features:
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
			// Add additional private modules here as needed.
		});

		// Uncomment the following lines if you want to include Slate UI even outside UMG usage:
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// If you plan to include plugins that need explicit enabling, add them in your uproject file.
		// For example, for Steam or other online subsystems:
		// "OnlineSubsystemSteam"

		// Optionally, you can add more modules or third-party libraries below based on additional project needs.
	}
}