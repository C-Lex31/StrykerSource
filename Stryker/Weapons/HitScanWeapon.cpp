// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Stryker/Character/StrykerCharacter.h"

//#include "Stryker/Components/WeaponComponent.h"
#include "Stryker/Stryker.h"
void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner()); //Owner is character 
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
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
		// End = PlayerCharacter->GetCrosshairHasObstacle() ? CenterEnd : CenterEnd;

		UWorld* World = GetWorld();
		if (World)
		{
			/*World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);*/
			UKismetSystemLibrary::LineTraceSingle(World, Start, End, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, FireHit, true, FColor::Blue, FColor::Green, 1.f);
			
			if (FireHit.bBlockingHit)
			{
				AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(FireHit.GetActor());
				if (StrykerCharacter && InstigatorController )
				{
					if (HasAuthority() )
					{
						UGameplayStatics::ApplyDamage(
							StrykerCharacter,
							Damage,
							InstigatorController,
							this,
							UDamageType::StaticClass()
						);
					}
				}
				if (ImpactParticles)
				{

					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}
			}
		}
	}
}
