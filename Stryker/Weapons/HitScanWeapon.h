// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API AHitScanWeapon : public AWeaponBase
{
	GENERATED_BODY()

	

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlashParticle;

	UPROPERTY(EditAnywhere)
	class USoundCue* FireSound;


	//UPROPERTY(EditAnywhere)
	//class UWeaponComponent*  WeaponComponent;
	class AStrykerCharacter* PlayerCharacter ;



protected:
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);



	UPROPERTY(EditAnywhere)
	class UParticleSystem* BodyImpactParticles;

	UPROPERTY(EditAnywhere)
	USoundCue* BodyHitSound;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* OtherImpactParticles;

	UPROPERTY(EditAnywhere)
	USoundCue* OtherHitSound;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* DefaultImpactParticles;

	UPROPERTY(EditAnywhere)
	USoundCue* DefaultHitSound;
public:
	virtual void Fire(const FVector& HitTarget) override;
	
};
