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

void AStrykerPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
}

void AStrykerPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	//Only update the HUD when Seconds change not every frame 
	if (CountdownInt != SecondsLeft)
	{
		SetMatchCountdown(MatchTime - GetServerTime());
	}

	CountdownInt = SecondsLeft;
}

void AStrykerPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RTT = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + 0.5f * RTT;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AStrykerPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfRequestReceive = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfRequestReceive);
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

void AStrykerPlayerController::SetWeaponAmmo_Implementation(int32 Ammo)
{
	bool bHUDValid =
		PlayerOverlay &&
		PlayerOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString WeaponAmmoString = FString::Printf(TEXT("%d"), Ammo);
		PlayerOverlay->WeaponAmmoAmount->SetText(FText::FromString(WeaponAmmoString));
	}
}

void AStrykerPlayerController::SetCarriedAmmo_Implementation(int32 CarriedAmmo)
{

	bool bHUDValid =
		PlayerOverlay &&
		PlayerOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString CarriedAmmoString = FString::Printf(TEXT("%d"), CarriedAmmo);
		PlayerOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoString));
	}
}

void AStrykerPlayerController::SetMatchCountdown(float CountdownTime)
{
	bool bHUDValid =
		PlayerOverlay &&
		PlayerOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString MatchCountdownString = FString::Printf(TEXT("%2d:%2d"), Minutes ,Seconds );
		PlayerOverlay->MatchCountdownText->SetText(FText::FromString(MatchCountdownString));
	}
}

void AStrykerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

}

float AStrykerPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AStrykerPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
	FTimerHandle TH_SyncTimeWithServer;
	GetWorldTimerManager().SetTimer(TH_SyncTimeWithServer, this, &ThisClass::CheckTimeSync , TimeSyncFrequency, true, -1.f);
}

void AStrykerPlayerController::CheckTimeSync()
{
	if (IsLocalController() )
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());	
	}
}
