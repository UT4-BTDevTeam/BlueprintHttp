
namespace UnrealBuildTool.Rules
{
	public class BlueprintHttp : ModuleRules
	{
		public BlueprintHttp(TargetInfo Target)
        {
            PrivateIncludePaths.Add("BlueprintHttp/Private");

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine",
					"Http",
					"Json",
					"JsonUtilities",
                    //"UnrealTournament",
				}
			);
		}
	}
}