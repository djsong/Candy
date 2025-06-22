// Copyright DJ Song Super-Star, Inc. All Rights Reserved.

#include "CandyModule.h"

#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "Candy"

DEFINE_LOG_CATEGORY(LogCandy);

void FCandyModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FCandyModule::OnPostEngineInit);

	UE_LOG(LogCandy, Log, TEXT("Candy Started"));
}

void FCandyModule::OnPostEngineInit()
{
	
	UE_LOG(LogCandy, Log, TEXT("Candy OnPostEngineInit"));
}

void FCandyModule::ShutdownModule()
{

	FCoreDelegates::OnPostEngineInit.RemoveAll(this);

	UE_LOG(LogCandy, Log, TEXT("Candy Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCandyModule, Candy)
