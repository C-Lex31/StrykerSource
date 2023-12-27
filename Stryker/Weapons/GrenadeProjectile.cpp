// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h "
#include "Stryker/Components/ThrowableMovementComponent.h"
AGrenadeProjectile::AGrenadeProjectile()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ThrowableMovementComponent = CreateDefaultSubobject<UThrowableMovementComponent>(TEXT("ThrowableMovementComponent"));
	ThrowableMovementComponent->bRotationFollowsVelocity = true;
	ThrowableMovementComponent->SetIsReplicated(true);
	ThrowableMovementComponent->bShouldBounce = true;

	SetReplicateMovement(true);
}


void AGrenadeProjectile::BeginPlay()
{
	Super::BeginPlay();

	SpawnTrailSystem();
	StartDestroyTimer();

	if(CollisionBox)
		CollisionBox->OnComponentHit.AddDynamic(this, &AGrenadeProjectile::OnProjectileHit);


}


void AGrenadeProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		ThrowableMovementComponent->SetFlagValue(true);
		return;
	}
	ThrowableMovementComponent->SetFlagValue(false);
	
}
void AGrenadeProjectile::Destroyed()
{
	ExplodeDamage();
	Super::Destroyed();
}