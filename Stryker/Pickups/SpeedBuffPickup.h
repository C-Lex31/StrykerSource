// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "SpeedBuffPickup.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API ASpeedBuffPickup : public APickupBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.f;

	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 850.f;

	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 15.f;
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
