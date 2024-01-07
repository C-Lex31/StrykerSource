// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Stryker/Stryker.h"


void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End =  TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		//UKismetSystemLibrary::LineTraceSingle(World, TraceStart, End, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, FireHit, true, FColor::Blue, FColor::Green, 1.f);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);

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
			
				AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(FireHit.GetActor());
				if (StrykerCharacter && HasAuthority() && InstigatorController )
				{
						UGameplayStatics::ApplyDamage(
							StrykerCharacter,
							Damage,
							InstigatorController,
							this,
							UDamageType::StaticClass()
						);
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
						FireHit.ImpactPoint
					);
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
