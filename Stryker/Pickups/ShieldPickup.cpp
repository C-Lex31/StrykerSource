// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickup.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "Stryker/Components/BuffComponent.h "

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(OtherActor);
	if (StrykerCharacter && StrykerCharacter->GetShield() != StrykerCharacter->GetMaxShield())
	{
		UBuffComponent* BC = StrykerCharacter->GetBuffComponent();
		if (BC)
		{
			BC->ReplenishShield(ShieldReplenishAmount, ShieldReplenishTime);
		}

		Destroy();
	}

}
