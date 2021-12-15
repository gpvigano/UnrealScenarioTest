// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class UnrealScenario : ModuleRules
{
	public UnrealScenario(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PublicIncludePaths.AddRange(new string[] { "UnrealScenario/Public" } );
		PrivateIncludePaths.AddRange(new string[] { "UnrealScenario/Private" } );

		//PublicSystemIncludePaths.Add( ModuleDirectory+"/../../../DigitalScenarioFramework/depend/include" );
        PublicSystemIncludePaths.Add( ModuleDirectory+ "/../../../DigitalScenarioFramework/DiScenFw/include");
        PublicAdditionalLibraries.Add(ModuleDirectory+ "/../../../DigitalScenarioFramework/DiScenFw/lib/vc140-x64-Release/DiScenFw-x64.lib");
        PublicDelayLoadDLLs.Add(ModuleDirectory+ "/../../../DigitalScenarioFramework/DiScenFw/lib/vc140-x64-Release/DiScenFw-x64.dll");

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
