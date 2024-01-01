// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta=(BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ShieldText;

	UPROPERTY(meta=(BindWidget))
	class UCrosshair* ObstacleCrosshair;

	UPROPERTY(meta=(BindWidget))
    UTextBlock* ScoreText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ScoreAmount;

	UPROPERTY(meta=(BindWidget))
    UTextBlock* DeathsText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* DeathCount;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* MatchCountdownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadeAmount;
	
	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingWarningImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingPulse;
};
