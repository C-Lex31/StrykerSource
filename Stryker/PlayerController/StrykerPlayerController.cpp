// Fill out your copyright notice in the Description page of Project Settings.


#include "StrykerPlayerController.h"
#include "Stryker/UI/HUD/StrykerHUD.h"
#include "Stryker/UI/HUD/CharacterOverlay.h"
#include "Stryker/UI/HUD/Announcement.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Stryker/Interfaces/PlayerControllerInterface.h"
#include "Stryker/PlayerState/StrykerPlayerState.h"
#include "Stryker/StrykerGameMode.h"
#include "Net/UnrealNetwork.h"
void AStrykerPlayerController::BeginPlay()
{
	Super::BeginPlay();
	StrykerHUD = Cast<AStrykerHUD>(GetHUD());
	ServerCheckMatchState();
}

void AStrykerPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

}

void AStrykerPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStrykerPlayerController, m_MatchState);
}

void AStrykerPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (m_MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (m_MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	//Only update the HUD when Seconds change not every frame 
	if (CountdownInt != SecondsLeft )
	{
		if (m_MatchState == MatchState::WaitingToStart)
			SetMatchWarmupCountdown(TimeLeft);

		if( m_MatchState == MatchState::InProgress)
			SetMatchCountdown(TimeLeft);
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

	if (bHUDValid )
	{
		const float HealthPercent = Health / MaxHealth;
		PlayerOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		PlayerOverlay->HealthText->SetText(FText::FromString(HealthString));
	}
}
void AStrykerPlayerController::SetShield_Implementation(float Shield, float MaxShield)
{
	bool bHUDValid =
		PlayerOverlay &&
		PlayerOverlay->ShieldBar &&
		PlayerOverlay->ShieldText;

	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		PlayerOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		PlayerOverlay->ShieldText->SetText(FText::FromString(ShieldString));
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

void AStrykerPlayerController::SetGrenadeAmmo_Implementation(int32 Count)
{
	bool bHUDValid =
		PlayerOverlay &&
		PlayerOverlay->GrenadeAmount;

	if (bHUDValid)
	{
		FString GrenadeCountString = FString::Printf(TEXT("%d"), Count);
		PlayerOverlay->GrenadeAmount->SetText(FText::FromString(GrenadeCountString));
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
		if(PlayerOverlay && PlayerOverlay->MatchCountdownText)
		PlayerOverlay->MatchCountdownText->SetText(FText::FromString(MatchCountdownString));
	}
}

void AStrykerPlayerController::SetMatchWarmupCountdown(float CountdownTime)
{
	bool bHUDValid =
		Announcement &&
		Announcement->WarmupTimer;

	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString MatchCountdownString = FString::Printf(TEXT("%2d:%2d"), Minutes, Seconds);
		if (Announcement)
			Announcement->WarmupTimer->SetText(FText::FromString(MatchCountdownString));
	}
}

void AStrykerPlayerController::MatchStateSet(FName State)
{
	m_MatchState = State;
	
	
if (m_MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}

}

void AStrykerPlayerController::HandleMatchHasStarted()
{
	if (Announcement)
	{
		UKismetSystemLibrary::PrintString(GetWorld());
		Announcement->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AStrykerPlayerController::OnRep_MatchState()
{
	
		if (m_MatchState == MatchState::InProgress)
		{
			HandleMatchHasStarted();
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

	GetWorldTimerManager().SetTimer(TH_SyncTimeWithServer, this, &ThisClass::CheckTimeSync , TimeSyncFrequency, true, -1.f);
	GetWorldTimerManager().SetTimer(TH_CheckPing, this, &ThisClass::CheckPing, CheckPingFrequency, true, -1.f);
}

void AStrykerPlayerController::CheckTimeSync()
{
	if (IsLocalController() )
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());	
	}
}

void AStrykerPlayerController::HighPingWarning()
{
	bool bHUDValid =
		PlayerOverlay &&
		PlayerOverlay->HighPingWarningImage &&
		PlayerOverlay->HighPingPulse;

	if (bHUDValid)
	{
		PlayerOverlay->PlayAnimation(PlayerOverlay->HighPingPulse,0.f,5);
	}
}

void AStrykerPlayerController::StopHighPingWarning()
{
	bool bHUDValid =
		PlayerOverlay &&
		PlayerOverlay->HighPingWarningImage &&
		PlayerOverlay->HighPingPulse;
	if (bHUDValid)
	{
		PlayerOverlay->StopAnimation(PlayerOverlay->HighPingPulse);
	}

}

void AStrykerPlayerController::CheckPing()
{
	PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
	if (PlayerState)
	{
		if (PlayerState->GetPing() * 4 > HighPingThreshold) // ping is compressed; it's actually ping / 4
		{
			HighPingWarning();
			PingAnimationRunningTime = 0.f;
		}
	}

	bool bHighPingAnimationPlaying =
		PlayerOverlay &&
		PlayerOverlay->HighPingPulse &&
		PlayerOverlay->IsAnimationPlaying(PlayerOverlay->HighPingPulse);

	if (bHighPingAnimationPlaying)
	{
		GetWorldTimerManager().SetTimer(TH_HighPingPulseDuration, this, &ThisClass::StopHighPingWarning, HighPingDuration, false, -1.f);
	}

}

void AStrykerPlayerController::ClientJoinMidGame_Implementation(FName State , float Warmup , float MatchDuration , float LevelStart)
{
	WarmupTime = Warmup;
	MatchTime = MatchDuration;
	LevelStartingTime = LevelStart;
	m_MatchState = State;
	MatchStateSet(m_MatchState);
	if (m_MatchState == MatchState::WaitingToStart)
	{
		Announcement = CreateWidget<UAnnouncement>(UGameplayStatics::GetPlayerController(GetWorld(), 0), AnnouncementClass);
		if (Announcement)
			Announcement->AddToViewport();
	}

}

void AStrykerPlayerController::ServerCheckMatchState_Implementation()
{
	AStrykerGameMode* StrykerGM = Cast<AStrykerGameMode>(UGameplayStatics::GetGameMode(this));
	if (StrykerGM)
	{
		WarmupTime = StrykerGM->WarmupTime;
		MatchTime = StrykerGM->MatchTime;
		LevelStartingTime = StrykerGM->LevelStartingTime;
		m_MatchState = StrykerGM->GetMatchState();
		ClientJoinMidGame(m_MatchState, WarmupTime, MatchTime, LevelStartingTime);
		#if 0
if (m_MatchState == MatchState::WaitingToStart)
		{
			Announcement = CreateWidget<UAnnouncement>(UGameplayStatics::GetPlayerController(GetWorld(), 0), AnnouncementClass);
			if (Announcement)
				Announcement->AddToViewport();
		}
#endif
	}
}


