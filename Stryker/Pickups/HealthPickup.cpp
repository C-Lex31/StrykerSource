// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "Stryker/Components/BuffComponent.h "
AHealthPickup::AHealthPickup()
{
	bReplicates = true;
}


void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(OtherActor);
	if (StrykerCharacter && StrykerCharacter->GetHealth()!= StrykerCharacter->GetMaxHealth())
	{
		UBuffComponent* BC = StrykerCharacter->GetBuffComponent();
		if (BC)
		{
			BC->Heal(HealAmount, HealingTime);
		}

		Destroy();
	}

	
}

