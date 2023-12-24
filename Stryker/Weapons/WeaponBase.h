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
	Initial ,
	Equipped,
	Dropped
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
	void ShowPickupWidget(bool bPickupWidget);
	void SetWeaponState(EWeaponState State);
	void SetHUDAmmo(); //Also called on server from weapon component
	FORCEINLINE bool  GetIsEmpty() { return Ammo <= 0.f; }
	FORCEINLINE bool  GetIsFull() { return Ammo == MagCapacity; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE EWeaponType GetWeaponType()const { return WeaponType; }
	FORCEINLINE uint32 GetAmmo()const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() { return MagCapacity; }
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
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	USkeletalMeshComponent* WeaponMesh;

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



	class AStrykerCharacter* OwnerCharacter;
	class AStrykerPlayerController* OwnerController;
	
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_Ammo)
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
};
