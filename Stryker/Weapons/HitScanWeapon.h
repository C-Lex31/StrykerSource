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
	class UParticleSystem* ImpactParticles;
	//UPROPERTY(EditAnywhere)
	//class UWeaponComponent*  WeaponComponent;
	class AStrykerCharacter* PlayerCharacter ;
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
public:
	virtual void Fire(const FVector& HitTarget) override;
	
};
