// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "StrykerGameMode.generated.h"

UCLASS(minimalapi)
class AStrykerGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AStrykerGameMode();
	virtual void PlayerEliminated(class AStrykerCharacter* EliminatedPlayer , class AStrykerPlayerController* PC_Victim , AStrykerPlayerController* PC_Attacker);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
};



