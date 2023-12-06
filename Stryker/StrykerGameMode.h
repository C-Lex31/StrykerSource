// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "StrykerGameMode.generated.h"

UCLASS(minimalapi)
class AStrykerGameMode : public AGameMode
{
	GENERATED_BODY()

		FTimerHandle TH_WarmUp;
	float CountdownTime = 0.f;
	float LevelStartingTime = 0.f;
	void UpdateCountdown();
public:
	AStrykerGameMode();
	virtual void PlayerEliminated(class AStrykerCharacter* EliminatedPlayer , class AStrykerPlayerController* PC_Victim , AStrykerPlayerController* PC_Attacker);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};



