// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StrykerPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API AStrykerPlayerController : public APlayerController
{
	GENERATED_BODY()
	class AStrykerHUD* StrykerHUD;
	class UCharacterOverlay* PlayerOverlay;
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category="UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget>PlayerOverlayClass;
protected:
	virtual void BeginPlay() override;
public:
	UFUNCTION(Client ,Reliable)
	void ClientHUD();
	UFUNCTION(Client ,Reliable)
	void SetHealth(float Health, float MaxHealth);
	UFUNCTION(Client ,Reliable)
	void SetScoreAmount(float Score);
	UFUNCTION(Client ,Reliable)
	void SetDeathCount(float Deaths);
	UFUNCTION(Client ,Reliable)
	void SetWeaponAmmo(int32 Ammo);
	UFUNCTION(Client ,Reliable)
	void SetCarriedAmmo(int32 CarriedAmmo);
	UFUNCTION(Client ,Reliable)
	void InitializeGameHUD();
	virtual void OnPossess(APawn* InPawn) override;
	FORCEINLINE UCharacterOverlay* GetGameHUD() { return PlayerOverlay; }
	bool bomba = false;
};
