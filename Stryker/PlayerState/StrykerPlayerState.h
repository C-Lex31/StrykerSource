// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "StrykerPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API AStrykerPlayerState : public APlayerState
{
	GENERATED_BODY()

	class AStrykerPlayerController* StrykerController;
	int32 Deaths;
public:
	//virtual void OnRep_Score()override;
	void AddToScore(float ScoreAmount);
	void AddToDeaths(int32 DeathCount);
};
