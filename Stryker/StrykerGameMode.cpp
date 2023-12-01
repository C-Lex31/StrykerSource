// Copyright Epic Games, Inc. All Rights Reserved.

#include "StrykerGameMode.h"
#include "Character/StrykerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Stryker/PlayerController/StrykerPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

AStrykerGameMode::AStrykerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/StrykerGame/Blueprints/Character/BP_StrykerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AStrykerGameMode::PlayerEliminated(AStrykerCharacter* EliminatedPlayer, AStrykerPlayerController* PC_Victim, AStrykerPlayerController* PC_Attacker)
{
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
