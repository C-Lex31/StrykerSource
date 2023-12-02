// Fill out your copyright notice in the Description pa


#include "StrykerPlayerState.h"
#include "Stryker/PlayerController/StrykerPlayerController.h "

#if 0
void AStrykerPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	StrykerController = StrykerController == nullptr ? Cast<AStrykerPlayerController>(GetPawn()->GetController()) : StrykerController;
	if (StrykerController)
	{
		StrykerController->SetScoreAmount(Score);
	}
}
#endif

//------Called from Game mode on server ------------------
void AStrykerPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(Score + ScoreAmount);
	StrykerController = StrykerController == nullptr ? Cast<AStrykerPlayerController>(GetPawn()->GetController()) : StrykerController;
	if (StrykerController)
	{
		StrykerController->SetScoreAmount(Score);// Client RPC
	}
	ForceNetUpdate();
}

void AStrykerPlayerState::AddToDeaths(int32 DeathCount)
{
	Deaths += DeathCount;
	StrykerController = StrykerController == nullptr ? Cast<AStrykerPlayerController>(GetPawn()->GetController()) : StrykerController;
	if (StrykerController)
	{
		StrykerController->SetDeathCount(Deaths);// Client RPC
	}
	ForceNetUpdate();
}
//-------------------------------------------------------------