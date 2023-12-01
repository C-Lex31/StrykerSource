// Fill out your copyright notice in the Description page of Project Settings.


#include "GM_Lobby.h"
#include "GameFramework/GameStateBase.h"

AGM_Lobby::AGM_Lobby()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/StrykerGame/Blueprints/Character/BP_StrykerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AGM_Lobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	int32 NumOfPlayers = GameState.Get()->PlayerArray.Num(); //GameState->PlayerArray.Num();
	if (NumOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString(TEXT("/Game/StrykerGame/Maps/MAP_Game?listen")));
		}
	}

}
