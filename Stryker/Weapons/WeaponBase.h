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
	void ShowPickupWidget(bool bPickupWidget);
	void SetWeaponState(EWeaponState State);
	void SetHUDAmmo(); //Also called on server from weapon component
	FORCEINLINE bool  GetIsEmpty() { return Ammo <= 0.f; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE EWeaponType GetWeaponType()const { return WeaponType; }
	/**
	* Textures for the weapon crosshairs
	*/

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;
	/** 
	* Zoomed FOV while aiming
	*/

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;
	/** 
	* Automatic fire
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .15f;
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;
	//void SetPlayerRef (class AStrykerCharacter* PlayerRef);
	//FORCEINLINE AStrykerCharacter* GetPlayerRef() { return PlayerCharacter; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
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
	 USkeletalMeshComponent* WeaponMesh;
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
	
	UPROPERTY(EditAnywhere, Replicated)
	int32 Ammo;

	//UFUNCTION()
	//void OnRep_Ammo();

	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
};
