// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedBuffPickup.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "Stryker/Components/BuffComponent.h "

void ASpeedBuffPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//CALLED ON SERVER ONLY
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(OtherActor);
	if (StrykerCharacter)
	{
		UBuffComponent* BC = StrykerCharacter->GetBuffComponent();
		if (BC)
		{
			BC->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}

		Destroy();
	}


}
