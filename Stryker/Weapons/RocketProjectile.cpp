// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectile.h"
#include "Kismet/GameplayStatics.h"


ARocketProjectile::ARocketProjectile()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ARocketProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* InstigatorPawn = GetInstigator();
	if (InstigatorPawn)
	{
		AController* InstigatorController = InstigatorPawn->GetController();
		if (InstigatorController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, Damage, 10.f, GetActorLocation(), DamageInnerRadius, DamageOuterRadius, 1.f, UDamageType::StaticClass(), TArray<AActor*>(), this);

		}
	}
	Super::OnProjectileHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

