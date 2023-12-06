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
	virtual void Tick(float DeltaTime) override;
	void SetHUDTime();

	/**
	* Sync time between client and server
	*/
	// SERVER RPC (Called on client to run on server) Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// CLIENT RPC - Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	void CheckTimeSync();
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
	//UFUNCTION(Client ,Reliable)
	void SetMatchCountdown(float CountdownTime);
	UFUNCTION(Client ,Reliable)
	void InitializeGameHUD();
	virtual void OnPossess(APawn* InPawn) override;
	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible
	FORCEINLINE UCharacterOverlay* GetGameHUD() { return PlayerOverlay; }
	float MatchTime = 120.f;//temp
	uint32 CountdownInt = 0;
};
