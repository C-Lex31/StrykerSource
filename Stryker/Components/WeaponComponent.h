// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Stryker/UI/HUD/StrykerHUD.h"
#include "Stryker/Enumerations/WeaponTypes.h"
#include "WeaponComponent.generated.h"

#define TRACE_LENGTH 80000.f

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
		UPROPERTY(EditAnywhere, Category = Weapon)
	    class UAnimMontage* FireWeaponMontage;
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

	// Carried ammo for the currently-equipped weapon
	UPROPERTY(Replicated)
	int32 CarriedAmmo;
	TMap<EWeaponType, int32> CarriedAmmoMap;
	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;
	void InitializeCarriedAmmo();
public:	
	// Sets default values for this component's properties
	UWeaponComponent();
	friend class AStrykerCharacter ;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	void EquipWeapon(AWeaponBase* WeaponToEqip);
	void FireButtonPressed(bool bPressed);
	float CrosshairShootingFactor;
//	FORCEINLINE float GetCrosshairShootingFactor() { return CrosshairShootingFactor; }
	FORCEINLINE void SetHitTarget(FVector Target) { HitTarget = Target; }
	void Fire();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void SetAiming(bool bAiming);

	UFUNCTION(Server , Reliable  )
	void ServerSetAiming(bool bAiming);
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION()
	void OnRep_EquipWeapon();

	void TraceCameraAim(FHitResult& TraceHitResult);
	void CrosshairLogicUpdate();
	void PlayFireMontage(bool bAiming);
	void SetHUDCrosshairs(float DeltaTime);
};
