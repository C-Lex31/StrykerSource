// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GM_Lobby.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API AGM_Lobby : public AGameMode
{
	GENERATED_BODY()
public :
	AGM_Lobby();

	virtual void PostLogin(APlayerController* NewPlayer) override;
};
