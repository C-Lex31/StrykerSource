// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API AHealthPickup : public APickupBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere ,Category ="PickupBaseProperties|Child")
	float HealAmount = 100.f;

	UPROPERTY(EditAnywhere, Category = "PickupBaseProperties|Child")
	float HealingTime = 5.f;

public:
	AHealthPickup();

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	)override;
};
