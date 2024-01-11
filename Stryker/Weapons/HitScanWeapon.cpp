// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "Stryker/PlayerController/StrykerPlayerController.h"
#include "Stryker/Components/LagCompensationComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Stryker/Stryker.h"


void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	FCollisionQueryParams CQP;
	CQP.bReturnPhysicalMaterial = true;
	if (World)
	{
		FVector End =  TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility,
			CQP
		);
		//UKismetSystemLibrary::LineTraceSingle(World, TraceStart, End, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, FireHit, true, FColor::Blue, FColor::Green, 1.f);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		//DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner()); //Owner is character 
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();// NOTE : Instigator controller will be null on all simulated proxies 
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AStrykerCharacter>(OwnerPawn) : PlayerCharacter;
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	if ( PlayerCharacter && MuzzleSocket )
	{
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
		FHitResult FireHit;
		TArray<AActor*>ActorsToIgnore;
		ActorsToIgnore.Add(OwnerPawn);
		ActorsToIgnore.Add((this));
		FVector SocketStart = SocketTransform.GetLocation();

	   //	FVector CenterEnd = CenterStart + CrosshairWorldDirection * 80000.f;
		//	FVector End = Start + UKismetMathLibrary::GetForwardVector(HitTarget) * 80000.f;
		FVector Start = PlayerCharacter->GetCrosshairHasObstacle() ? SocketStart : SocketStart;
		//if (World)
		//{
			/*World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);*/
			//UKismetSystemLibrary::LineTraceSingle(World, Start, End, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, FireHit, true, FColor::Blue, FColor::Green, 1.f);
			WeaponTraceHit(Start, HitTarget, FireHit);
			//int surfaceInt = UKismetMathLibrary::Conv_ByteToInt(FireHit.PhysMaterial->SurfaceType);

				AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(FireHit.GetActor());
				if (StrykerCharacter  && InstigatorController )
				{
					bool bCauseAuthoritativeDamage = !bUseSSR || OwnerPawn->IsLocallyControlled();
					if (HasAuthority() && bCauseAuthoritativeDamage)
					{
						const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
						UGameplayStatics::ApplyDamage(
							StrykerCharacter,
							DamageToCause,
							InstigatorController,
							this,
							UDamageType::StaticClass()
						);
					}
					if (!HasAuthority() && bUseSSR)
					{
						OwnerCharacter = OwnerCharacter == nullptr ? Cast<AStrykerCharacter>(OwnerPawn) : OwnerCharacter;
						OwnerController = OwnerController == nullptr ? Cast<AStrykerPlayerController>(InstigatorController) : OwnerController;
						if (OwnerController && OwnerCharacter && OwnerCharacter->GetLCC()&& OwnerCharacter->IsLocallyControlled())
						{
							OwnerCharacter->GetLCC()->ServerScoreRequest(
								StrykerCharacter,
								Start,
								HitTarget,
								OwnerController->GetServerTime() - OwnerController->SingleTripTime,
								this
							);
						}
					
					}

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
				

			
			if (MuzzleFlashParticle)
			{
				UGameplayStatics::SpawnEmitterAttached(
					MuzzleFlashParticle,
				    WeaponMesh,
					FName{},
					SocketTransform.GetLocation(),
					FRotator(SocketTransform.GetRotation()),
					EAttachLocation::KeepWorldPosition
				);
			}
			if (FireSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					FireSound,
					GetActorLocation()
				);
			}

	}
}
