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

	FTimerHandle TH_DestroyTrail;
	void DestroyTrailParticle();
	
	UPROPERTY(EditAnywhere , meta = (AllowPrivateAccess ="true"))
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float DamageOuterRadius = 500.f;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailParticle;

	UPROPERTY();
	class UNiagaraComponent* TrailParticleComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;

	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

protected:
	virtual void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay()override;
public:

	ARocketProjectile() ;
	void Destroyed() override;
};
