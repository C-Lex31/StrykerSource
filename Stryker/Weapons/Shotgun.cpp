// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Stryker/Character/StrykerCharacter.h"
void AShotgun::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	if (MuzzleSocket)
	{
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		TMap<AStrykerCharacter*, uint32> HitMap;
		for(uint32 i=0;i<NumberOfPellets;i++)
		{ 
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(FireHit.GetActor());

			if (StrykerCharacter && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(StrykerCharacter))
				{
					HitMap[StrykerCharacter]++;
				}
				else
				{
					HitMap.Emplace(StrykerCharacter, 1);
				}
			}

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					.5f,
					FMath::FRandRange(-.5f, .5f)
				);
			}
		}

		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value, //If Player get hits 2 times , dmg will be 2 times more 
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}

		
	}
}
