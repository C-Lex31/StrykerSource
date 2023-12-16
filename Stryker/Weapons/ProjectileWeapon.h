// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API AProjectileWeapon : public AWeaponBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere )
	TSubclassOf<class AProjectile>ProjectileClass;

public:
	virtual void Fire(const FVector& HitTarget) override;
};
