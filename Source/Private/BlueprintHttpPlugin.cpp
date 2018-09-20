
#include "ModuleManager.h"
#include "ModuleInterface.h"

class FBlueprintHttpPlugin : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FBlueprintHttpPlugin, BlueprintHttp )

//================================================
// Startup
//================================================

void FBlueprintHttpPlugin::StartupModule()
{
	UE_LOG(LogLoad, Log, TEXT("[BlueprintHttp] StartupModule"));
}

//================================================
// Shutdown
//================================================

void FBlueprintHttpPlugin::ShutdownModule()
{
	UE_LOG(LogLoad, Log, TEXT("[BlueprintHttp] ShutdownModule"));
}
