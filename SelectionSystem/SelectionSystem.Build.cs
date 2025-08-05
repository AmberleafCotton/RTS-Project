// Copyright 2025 AmberleafCotton. All right reserved.

using UnrealBuildTool;

public class SelectionSystem : ModuleRules
{
	public SelectionSystem(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Specify public include paths (empty array with explicit type).
		PublicIncludePaths.AddRange(new string[] { });

		// Specify private include paths (empty array with explicit type).
		PrivateIncludePaths.AddRange(new string[] { });

		// Declare public dependencies that this module statically links with.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"UMG", "FinalRTS", "EnhancedInput" // For UI and widget support.
			
			// Add any additional public dependencies here.
		});

		// Declare private dependencies that this module statically links with.
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore"  // For low-level UI functionality.
			// Add any additional private dependencies here.
		});

		// List any modules that your module loads dynamically.
		DynamicallyLoadedModuleNames.AddRange(new string[] { });
	}
}