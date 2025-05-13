// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// ReSharper disable once InconsistentNaming
public class IkarusTheCompanion : ModuleRules
{
    public IkarusTheCompanion(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        // Public include paths – these are visible to any module that depends on this plugin.
        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
            }
        );
                
        // Private include paths – these are for internal use only.
        PrivateIncludePaths.AddRange(
            new string[] {
                // ... add other private include paths required here ...
            }
        );
        
        // Public dependencies: Making sure the plugin has access to core functionalities such as Engine, AI, UI, and JSON.
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "EnhancedInput",
                "AIModule",
                "GameplayTasks",
                "GameplayAbilities",
                "GameFeatures",
                "GameplayTags",
                "NavigationSystem",
                // "EnvironmentQueryEditor", // Removed from here
                "CommonUI",
                "UMG",
                "Slate",
                "SlateCore",
                "Json",
                "JsonUtilities"
            }
        );
        
        // Add editor-only modules conditionally
        if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "EnvironmentQueryEditor"
                }
            );
        }
            
        // Private dependencies: Add any modules that you only need internally.
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                // If additional private modules are needed, add them here.
                // For example, if you implement custom logging or networking features:
                // "OnlineSubsystem",
                // "OnlineSubsystemUtils"
            }
        );
        
        // Dynamically loaded modules: Include any modules that your plugin loads at runtime.
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
        );
    }
}