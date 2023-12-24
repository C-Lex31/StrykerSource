// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Stryker/UI/HUD/StrykerHUD.h"
#include "Stryker/Enumerations/WeaponTypes.h"
#include "Stryker/Enumerations/CombatState.h"
#include "WeaponComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STRYKER_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

		class AStrykerPlayerController* StrykerPlayerController;
	    class AStrykerHUD* HUD;
		UPROPERTY(ReplicatedUsing = OnRep_EquipWeapon)
		class AWeaponBase* EquippedWeapon; 
	    class AStrykerCharacter* PlayerCharacter;
		//ACharacter* PlayerCharacter;
		UPROPERTY(Replicated)
		bool bIsAiming;
		UPROPERTY(EditAnywhere)
		float BaseWalkSpeed;
		UPROPERTY(EditAnywhere)
		float AimWalkSpeed;
		bool bFireButtonPressed;

		FVector CameraTraceEndLocation;
		/**
	* HUD and crosshairs
	*/
		float CrosshairVelocityFactor;
		float CrosshairInAirFactor;
		float CrosshairAimFactor;
		
		FVector HitTarget;
		FHUDPackage HUDPackage;
		/** 
	* Aiming and FOV
	*/

	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	//float ZoomedFOV = 30.f;
	float CurrentFOV;
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;
	void InterpFOV(float DeltaTime);
	/**
	* Automatic fire
	*/
	FTimerHandle FireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();
	bool CanFire();

	UPROPERTY( ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied ;
	UFUNCTION()
	void OnRep_CombatState();

	// Carried ammo for the currently-equipped weapon
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	TMap<EWeaponType, int32> CarriedAmmoMap;
	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500.f;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 150.f;

	UPROPERTY(EditAnywhere)
	int32 StartingRLAmmo = 0.f;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 175.f;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 30.f;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 25.f;

	void InitializeCarriedAmmo();
	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
	int32 AmountToReload();
	
public:	
	// Sets default values for this component's properties
	UWeaponComponent();
	friend class AStrykerCharacter ;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	void EquipWeapon(AWeaponBase* WeaponToEqip);
	void FireButtonPressed(bool bPressed);
	void JumpToShotgunEnd();

	float CrosshairShootingFactor;
//	FORCEINLINE float GetCrosshairShootingFactor() { return CrosshairShootingFactor; }
	FORCEINLINE void SetHitTarget(FVector Target) { HitTarget = Target; }
	void Fire();
	void Reload();
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void SetAiming(bool bAiming);

	UFUNCTION(Server , Reliable  )
	void ServerSetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server , Reliable)
	void ServerReload();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION()
	void OnRep_EquipWeapon();

	UFUNCTION()
	void OnRep_CarriedAmmo();

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void HandleReload();
	void TraceCameraAim(FHitResult& TraceHitResult);
	void CrosshairLogicUpdate();
	void SetHUDCrosshairs(float DeltaTime);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeaponBase* WeaponToEquip);
};
