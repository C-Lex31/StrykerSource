// Fill out your copyright notice in the Description page of Project Settings.


#include "StrykerPlayerController.h"
#include "Stryker/UI/HUD/StrykerHUD.h"
#include "Stryker/UI/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Stryker/Interfaces/PlayerControllerInterface.h"
void AStrykerPlayerController::BeginPlay()
{
	Super::BeginPlay();
	StrykerHUD = Cast<AStrykerHUD>(GetHUD());
}

//Called on owning clients only
void AStrykerPlayerController::ClientHUD_Implementation()
{

	
}

void AStrykerPlayerController::SetHealth_Implementation(float Health, float MaxHealth)
{
	bool bHUDValid =
		PlayerOverlay &&
		PlayerOverlay->HealthBar &&
		PlayerOverlay->HealthText;

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		PlayerOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		PlayerOverlay->HealthText->SetText(FText::FromString(HealthString));
	}
}

void AStrykerPlayerController::InitializeGameHUD_Implementation()
{
	PlayerOverlay = CreateWidget<UCharacterOverlay>(UGameplayStatics::GetPlayerController(GetWorld(), 0), PlayerOverlayClass);
	//if(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	 //Cast<IPlayerControllerInterface>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->I_ClientHUD();
	if(PlayerOverlay)
		PlayerOverlay->AddToViewport();
}

void AStrykerPlayerController::SetScoreAmount_Implementation(float Score)
{
	bool bHUDValid =
		PlayerOverlay &&
		PlayerOverlay->ScoreText &&
		PlayerOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreString = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		PlayerOverlay->ScoreAmount->SetText(FText::FromString(ScoreString));
	}
}

void AStrykerPlayerController::SetDeathCount_Implementation(float Deaths)
{
	bool bHUDValid =
		PlayerOverlay &&
		PlayerOverlay->DeathsText &&
		PlayerOverlay->DeathCount;

	if (bHUDValid)
	{
		FString DeathsString = FString::Printf(TEXT("%d"), FMath::FloorToInt(Deaths));
		PlayerOverlay->DeathCount->SetText(FText::FromString(DeathsString));
	}
}

void AStrykerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

}

