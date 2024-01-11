// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "Stryker/Components/LagCompensationComponent.h"
#include "Stryker/PlayerController/StrykerPlayerController.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeaponBase::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	if (MuzzleSocket)
	{
		const FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		// Maps hit character to number of times hit
		TMap<AStrykerCharacter*, uint32> HitMap;
		TMap<AStrykerCharacter*, uint32> HeadShotHitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(FireHit.GetActor());
			if (StrykerCharacter)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");

				if (bHeadShot)
				{
					if (HeadShotHitMap.Contains(StrykerCharacter)) HeadShotHitMap[StrykerCharacter]++;
					else HeadShotHitMap.Emplace(StrykerCharacter, 1);
				}
				else
				{
					if (HitMap.Contains(StrykerCharacter)) HitMap[StrykerCharacter]++;
					else HitMap.Emplace(StrykerCharacter, 1);
				}


				if (FireHit.PhysMaterial != nullptr)
				{
					switch (FireHit.PhysMaterial->SurfaceType)
					{
					case EPhysicalSurface::SurfaceType1:
						if (BodyImpactParticles)
						{

							UGameplayStatics::SpawnEmitterAtLocation(
								GetWorld(),
								BodyImpactParticles,
								FireHit.ImpactPoint,
								FireHit.ImpactNormal.Rotation()
							);
						}
						if (BodyHitSound)
						{
							UGameplayStatics::PlaySoundAtLocation(
								this,
								BodyHitSound,
								FireHit.ImpactPoint
							);
						}

						break;

					case EPhysicalSurface::SurfaceType2:

						if (OtherImpactParticles)
						{

							UGameplayStatics::SpawnEmitterAtLocation(
								GetWorld(),
								OtherImpactParticles,
								FireHit.ImpactPoint,
								FireHit.ImpactNormal.Rotation()
							);
						}
						if (OtherHitSound)
						{
							UGameplayStatics::PlaySoundAtLocation(
								this,
								OtherHitSound,
								FireHit.ImpactPoint
							);
						}
						break;

					default:
						if (DefaultImpactParticles)
						{

							UGameplayStatics::SpawnEmitterAtLocation(
								GetWorld(),
								DefaultImpactParticles,
								FireHit.ImpactPoint,
								FireHit.ImpactNormal.Rotation()
							);
						}
						if (DefaultHitSound)
						{
							UGameplayStatics::PlaySoundAtLocation(
								this,
								DefaultHitSound,
								FireHit.ImpactPoint
							);
						}
						break;
					}
				}
				
			}
		}
		TArray<AStrykerCharacter*> HitCharacters;

		// Maps Character hit to total damage
		TMap<AStrykerCharacter*, float> DamageMap;

		// Calculate body shot damage by multiplying times hit x Damage - store in DamageMap
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key)
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);

				HitCharacters.AddUnique(HitPair.Key);
			}
		}

		// Calculate head shot damage by multiplying times hit x HeadShotDamage - store in DamageMap
		for (auto HeadShotHitPair : HeadShotHitMap)
		{
			if (HeadShotHitPair.Key)
			{
				if (DamageMap.Contains(HeadShotHitPair.Key)) DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
				else DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);

				HitCharacters.AddUnique(HeadShotHitPair.Key);
			}
		}

		// Loop through DamageMap to get total damage for each character
		for (auto DamagePair : DamageMap)
		{
			if (DamagePair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseSSR || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
						DamagePair.Key, // Character that was hit
						DamagePair.Value, // Damage calculated in the two for loops above
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}


		if (!HasAuthority() && bUseSSR)
		{
			OwnerCharacter = OwnerCharacter == nullptr ? Cast<AStrykerCharacter>(OwnerPawn) : OwnerCharacter;
			OwnerController = OwnerController == nullptr ? Cast<AStrykerPlayerController>(InstigatorController) : OwnerController;
			if (OwnerController && OwnerCharacter && OwnerCharacter->GetLCC() && OwnerCharacter->IsLocallyControlled())
			{
				OwnerCharacter->GetLCC()->ServerShotgunScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime
				);
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}
