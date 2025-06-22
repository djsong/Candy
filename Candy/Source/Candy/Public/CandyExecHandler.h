// Copyright DJ Song Super-Star. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Misc/CoreMisc.h"
#include "Http.h"

#if WITH_EDITOR

namespace Candy
{
	/** Configure message to let ChatGPT response in intended way - our commands */
	bool SendChatGPTCmdRequest(const FString& RequestMessage);

	/** Just send the message as is. */
	bool SendChatGPTSimpleMessage(const FString& Message);

	void OnChatGPTResponse(FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful);

	void HandleCmdAddStaticMeshActor(UWorld* InWorld, const TCHAR* Cmd);
	void HandleCmdMoveStaticMeshActor(UWorld* InWorld, const TCHAR* Cmd);
	void HandleCmdScaleStaticMeshActor(UWorld* InWorld, const TCHAR* Cmd);
}

#endif // WITH_EDITOR