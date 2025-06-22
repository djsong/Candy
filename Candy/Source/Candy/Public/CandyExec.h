// Copyright DJ Song Super-Star. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Misc/CoreMisc.h"

#if WITH_EDITOR

struct FCandyExec : public FSelfRegisteringExec
{
	FCandyExec();
	
public:
	static const FString CMD_ADD_STATICMESH_ACTOR;
	static const FString CMD_MOVE_STATICMESH_ACTOR;
	static const FString CMD_SCALE_STATICMESH_ACTOR;

protected:
	// Begin FExec Interface
	virtual bool Exec_Editor(UWorld* Inworld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	// End FExec Interface
};

#endif // WITH_EDITOR