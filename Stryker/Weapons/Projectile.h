// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class STRYKER_API AProjectile : public AActor
{
	GENERATED_BODY()



	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

public:	

	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileComponent;

	UFUNCTION()
	virtual void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	float Damage = 20, f;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailParticle;

	UPROPERTY();
	class UNiagaraComponent* TrailParticleComponent;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float DamageOuterRadius = 500.f;

	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();
};
