// Copyright DJ Song Super-Star. All Rights Reserved.

#include "CandyExec.h"
#include "CandyExecHandler.h"
#include "CandyModule.h"


#if WITH_EDITOR

FCandyExec CandyExec;

const FString FCandyExec::CMD_ADD_STATICMESH_ACTOR = FString(TEXT("CandyAddStaticMeshActor"));
const FString FCandyExec::CMD_MOVE_STATICMESH_ACTOR = FString(TEXT("CandyMoveStaticMeshActor"));
const FString FCandyExec::CMD_SCALE_STATICMESH_ACTOR = FString(TEXT("CandyScaleStaticMeshActor"));

FCandyExec::FCandyExec()
{
}

bool FCandyExec::Exec_Editor(UWorld* Inworld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	// For ChatGPT request
	if (FParse::Command(&Cmd, TEXT("Candy")))
	{
		// The main command to use.

		const FString FirstArg = FParse::Token(Cmd, false);

		Candy::SendChatGPTCmdRequest(FirstArg);

		return true;
	}
	else if (FParse::Command(&Cmd, TEXT("CandyHi")))
	{
		// Command to use when you just send any message to ChatGPT.

		const FString FirstArg = FParse::Token(Cmd, false);

		Candy::SendChatGPTSimpleMessage(FirstArg);

		return true;
	}
	// For ChatGPT response
	else if (FParse::Command(&Cmd, *CMD_ADD_STATICMESH_ACTOR))
	{
		Candy::HandleCmdAddStaticMeshActor(Inworld, Cmd);

		return true;
	}
	else if (FParse::Command(&Cmd, *CMD_MOVE_STATICMESH_ACTOR))
	{
		Candy::HandleCmdMoveStaticMeshActor(Inworld, Cmd);
		return true;
	}
	else if (FParse::Command(&Cmd, *CMD_SCALE_STATICMESH_ACTOR))
	{
		Candy::HandleCmdScaleStaticMeshActor(Inworld, Cmd);
		return true;
	}

	return false;
};


#endif // WITH_EDITOR
