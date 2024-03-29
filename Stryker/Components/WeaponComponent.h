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

		UPROPERTY(VisibleAnywhere , BlueprintReadOnly , ReplicatedUsing = OnRep_EquipWeapon , meta=(AllowPrivateAccess="true"))
		class AWeaponBase* EquippedWeapon; 

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly ,ReplicatedUsing = OnRep_SecondaryWeapon, meta=(AllowPrivateAccess = "true"))
		AWeaponBase* SecondaryWeapon;

	    class AStrykerCharacter* PlayerCharacter;
		
		UPROPERTY(ReplicatedUsing = OnRep_Aiming)
		bool bIsAiming;

		UPROPERTY(EditAnywhere)
		float BaseWalkSpeed;

		UPROPERTY(EditAnywhere)
		float AimWalkSpeed;

		bool bFireButtonPressed;

		bool bLocallyReloading = false;
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

	bool bAimButtonPressed = false;
	/**
	* Automatic fire
	*/
	
	UPROPERTY(VisibleAnywhere)
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();
	bool CanFire();

	UPROPERTY( VisibleAnywhere ,ReplicatedUsing = OnRep_CombatState)
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

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 10.f;
	
	UPROPERTY(Replicated)
	int32 Grenades = 6.f;

	void InitializeCarriedAmmo();
	void UpdateAmmoValues();
	void UpdateHUDGrenades();
	void UpdateShotgunAmmoValues();
	int32 AmountToReload();
	
public:	
	// Sets default values for this component's properties
	UWeaponComponent();
	FTimerHandle FireTimer;
	friend class AStrykerCharacter ;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	void EquipWeapon(AWeaponBase* WeaponToEquip);
	void SwapWeapons();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBackpack(AActor* ActorToAttach);
	void FireButtonPressed(bool bPressed);
	void JumpToShotgunEnd();

	float CrosshairShootingFactor;
//	FORCEINLINE float GetCrosshairShootingFactor() { return CrosshairShootingFactor; }
	FORCEINLINE void SetHitTarget(FVector Target) { HitTarget = Target; }
	FORCEINLINE int32 GetGrenadeCount() { return Grenades; }
	FORCEINLINE bool GetLocallyReloading() { return bLocallyReloading; }
	bool GetShouldSwapWeapons();
	void Fire();
	void Reload();
	void TossGrenade();
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishTossGrenade();

	UFUNCTION(BlueprintCallable)
	void BP_TossGrenade();

	UFUNCTION(BlueprintCallable)
	void BP_FinishEquip();

	UFUNCTION(BlueprintCallable)
	void BP_EquipAttachWeapon();



	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> SSR_GrenadeClass;
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

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server , Reliable)
	void ServerReload();

	UFUNCTION(Server, Reliable)
	void ServerTossGrenadeCosmetic();

	UFUNCTION(Server, Reliable)
	void ServerTossGrenade(const FVector_NetQuantize& Target);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastToss(const FVector_NetQuantize& Target);

	UFUNCTION(Server, Reliable)
	void ServerEquipAttachWeapon();

	UFUNCTION(Server, Reliable)
	void ServerFinishSwappingWeapon();

	UFUNCTION()
	void OnRep_EquipWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UFUNCTION()
	void OnRep_Aiming();

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	void LocalToss(const FVector_NetQuantize& Target);
	void HandleReload();
	void TraceCameraAim(FHitResult& TraceHitResult);
	void CrosshairLogicUpdate();
	void SetHUDCrosshairs(float DeltaTime);
	void UpdateCarriedAmmo();

	void PlayEquipWeaponSound(AWeaponBase* WeaponToEquip);
	void ShowAttachedGrenade(bool bShowGrenade);
	void EquipPrimaryWeapon(AWeaponBase* WeaponToEquip);
	void EquipSecondaryWeapon(AWeaponBase* WeaponToEquip);
	void TossGrenadeCosmetic();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();
};
