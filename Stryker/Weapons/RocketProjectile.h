// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "RocketProjectile.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API ARocketProjectile : public AProjectile
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere , meta = (AllowPrivateAccess ="true"))
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float DamageOuterRadius = 500.f;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;
protected:
	virtual void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
public:

	ARocketProjectile() ;
};
