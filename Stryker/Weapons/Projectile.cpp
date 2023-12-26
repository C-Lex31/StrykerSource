// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h "
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "Stryker/Stryker.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"


// Sets default values
AProjectile::AProjectile()   
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>("CollisionBox");
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);
	
	//ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//ProjectileComponent->bRotationFollowsVelocity = true;
}
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	APawn* InstigatorPawn = GetInstigator();
	FTimerHandle TH_ToggleCollison;
#if 0
	if (!InstigatorPawn)
		CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);
	else
	{
		CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Ignore);
		GetWorldTimerManager().SetTimer(
			TH_ToggleCollison, this, &ThisClass::ToggleCollision, 0.01f, false);
	}
#endif // 0

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnProjectileHit);
	}
}

void AProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(OtherActor);
	if (StrykerCharacter)
	{
	//	StrykerCharacter->MulticastHit();
	}
	Destroy();
}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::SpawnTrailSystem()
{

	if (TrailParticle)
	{
		TrailParticleComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailParticle,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

void AProjectile::ExplodeDamage()
{
	APawn* InstigatorPawn = GetInstigator();
	if (InstigatorPawn && HasAuthority())
	{
		AController* InstigatorController = InstigatorPawn->GetController();
		if (InstigatorController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, Damage, 10.f, GetActorLocation(), DamageInnerRadius, DamageOuterRadius, 1.f, UDamageType::StaticClass(), TArray<AActor*>(), this, InstigatorController);

		}
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

}

