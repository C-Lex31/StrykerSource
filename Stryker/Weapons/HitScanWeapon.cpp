// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Stryker/Stryker.h"
FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;

	if (bDrawDebugScatterTrace)
	{
		DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
		DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
		DrawDebugLine(
			GetWorld(),
			TraceStart,
			FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),// Dividing to prevent overflow of x,y,z values as TRACE_LENGTH is a large value.
			FColor::Cyan,
			true);
	}
	

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;

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
		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}
		FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		FVector CrosshairWorldPosition;
		FVector CrosshairWorldDirection;
		 UGameplayStatics::DeprojectScreenToWorld(
			UGameplayStatics::GetPlayerController(this, 0),
			CrosshairLocation,
			CrosshairWorldPosition,
			CrosshairWorldDirection);
		 FVector CenterStart = CrosshairWorldPosition;
		float DistanceToCharacter = (PlayerCharacter->GetActorLocation() - CenterStart).Size();
		CenterStart += CrosshairWorldDirection * (DistanceToCharacter + 100.f);

	   //	FVector CenterEnd = CenterStart + CrosshairWorldDirection * 80000.f;
		//	FVector End = Start + UKismetMathLibrary::GetForwardVector(HitTarget) * 80000.f;
		FVector Start = PlayerCharacter->GetCrosshairHasObstacle() ? SocketStart : SocketStart;
		FVector End = Start + (HitTarget - Start) * 1.25f;

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
