// Copyright DJ Song Super-Star, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CandySettings.generated.h"

/**
 * File: /Plugins/Candy/Config/BaseCandy.ini
 * Section: [/Script/Candy.CandySettings]
 */
UCLASS(config=Candy)
class UCandySettings : public UObject
{
	GENERATED_BODY()

public:
	UCandySettings(const FObjectInitializer& ObjectInitializer);

	/** What you need to get on your own to use this plugin. */
	UPROPERTY(Config)
	FString ChatGPTApiKey;
};

