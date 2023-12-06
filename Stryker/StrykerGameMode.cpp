// Copyright Epic Games, Inc. All Rights Reserved.

#include "StrykerGameMode.h"
#include "Character/StrykerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Stryker/PlayerController/StrykerPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Stryker/PlayerState/StrykerPlayerState.h"


AStrykerGameMode::AStrykerGameMode()
{

	bDelayedStart = true;
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/StrykerGame/Blueprints/Character/BP_StrykerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	
}

void AStrykerGameMode::BeginPlay()
{
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AStrykerGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
}


void AStrykerGameMode::PlayerEliminated(AStrykerCharacter* EliminatedPlayer, AStrykerPlayerController* PC_Victim, AStrykerPlayerController* PC_Attacker)
{
	AStrykerPlayerState* AttackerPS = PC_Attacker ? Cast<AStrykerPlayerState>(PC_Attacker->PlayerState) : nullptr;
	AStrykerPlayerState* VictimPS = PC_Victim ? Cast<AStrykerPlayerState>(PC_Victim->PlayerState) : nullptr;

	if (AttackerPS && AttackerPS != VictimPS)
	{
		AttackerPS->AddToScore(1.f);
	}
	if (VictimPS)
	{
		VictimPS->AddToDeaths(1);
	}
	if(EliminatedPlayer)
		EliminatedPlayer->ServerEliminated();
}

void AStrykerGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

