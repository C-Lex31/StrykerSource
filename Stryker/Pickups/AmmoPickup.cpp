// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "Stryker/Components/WeaponComponent.h "
void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(OtherActor);

	if (StrykerCharacter)
	{
		UWeaponComponent* WeaponComponent = StrykerCharacter->GetWeaponComponent();
		if (WeaponComponent)
		{
			WeaponComponent->PickupAmmo(WeaponType  , AmmoAmount); 
		}
	}
	Destroy();
}
