// Copyright DJ Song Super-Star. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Candy : ModuleRules
{
	public Candy(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Json",
				"Http",
				"AssetRegistry"
			}
			);


		PrecompileForTargets = PrecompileTargetsType.Any;
	}
}
