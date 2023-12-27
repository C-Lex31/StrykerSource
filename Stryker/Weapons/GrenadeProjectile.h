// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "GrenadeProjectile.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API AGrenadeProjectile : public AProjectile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;

	UPROPERTY(VisibleAnywhere)
	class UThrowableMovementComponent* ThrowableMovementComponent;
public:
	AGrenadeProjectile();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	virtual void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	
};
