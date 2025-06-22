// Copyright DJ Song Super-Star. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCandy, Log, All);

class FCandyModule final : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
	void OnPostEngineInit();

};