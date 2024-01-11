// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Stryker/Enumerations/WeaponTypes.h"
#include "WeaponBase.generated.h"

UENUM(BlueprintType)
enum class EWeaponState :uint8
{
	EWS_Initial ,
	EWS_EquippedPrimary,
	EWS_EquippedSecondary,
	EWS_Dropped
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class STRYKER_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	virtual void Fire(const FVector& HitTarget);
	void DropWeapon();
	void AddAmmo(int32 AmmoToAdd);
	FVector TraceEndWithScatter( const FVector& HitTarget);
	void ShowPickupWidget(bool bPickupWidget);
	void SetWeaponState(EWeaponState State);
	void SetHUDAmmo(); //Also called on server from weapon component
	FORCEINLINE bool  GetIsEmpty() { return Ammo <= 0.f; }
	FORCEINLINE bool  GetIsFull() { return Ammo == MagCapacity; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE EWeaponType GetWeaponType()const { return WeaponType; }
	FORCEINLINE uint32 GetAmmo()const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() { return MagCapacity; }
	FORCEINLINE FName GetRightHandSocket() { return RightHandSocket; }
	FORCEINLINE FName GetLeftHandSocket() { return LeftHandSocket; }
	FORCEINLINE FName GetHolsterSocket() { return HolsterSocket; }
	FORCEINLINE float GetWeaponDamage() { return Damage; }
	FORCEINLINE float GetHeadShotDamage() { return HeadShotDamage; }
	/**
	* Textures for the weapon crosshairs
	*/

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Crosshairs")
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Crosshairs")
	UTexture2D* CrosshairImage;


	/** 
	* Zoomed FOV while aiming
	*/

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	float ZoomInterpSpeed = 20.f;
	/** 
	* Automatic fire
	*/
	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	float FireDelay = .15f;
	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	bool bAutomatic = true;
	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	class USoundCue* EquipSound;

		/**
	* Enable or disable custom depth
	*/
	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnWeaponStateSet();
	virtual void OnEquippedPrimary();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();



	class AStrykerCharacter* OwnerCharacter;
	class AStrykerPlayerController* OwnerController;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		) ;

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex
		) ;
	
	/**
* Trace end with scatter
*/

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere)
	bool bDrawDebugScatterTrace = false;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere ,Category = "WeaponProperties")
	float HeadShotDamage = 40.f;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	bool bUseSSR = false;

private:

	UPROPERTY(VisibleAnywhere , Category="WeaponProperties")
	class USphereComponent* AreaSphere;
	UPROPERTY(ReplicatedUsing=OnRep_WeaponState, VisibleAnywhere, Category = "WeaponProperties")
	EWeaponState WeaponState;	
	UFUNCTION()
	void OnRep_WeaponState();
	UPROPERTY(VisibleAnywhere , Category="WeaponProperties")
	class UWidgetComponent* PickupWidget;
	UPROPERTY(EditAnywhere , Category="WeaponProperties")
	class UAnimationAsset* WeaponFireAnimation;


	 
	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|AttachmentSockets")
	FName RightHandSocket = FName("RightHandSocket");

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|AttachmentSockets")
	FName LeftHandSocket = FName("LeftHandSocket");

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|AttachmentSockets")
	FName HolsterSocket = FName("HolsterSocket");



	
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);


	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	// The number of unprocessed server requests for Ammo.
// Incremented in SpendRound for client , decremented in ClientUpdateAmmo.
	int32 Sequence = 0;
};
