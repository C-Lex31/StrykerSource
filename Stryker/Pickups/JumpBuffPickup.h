// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "JumpBuffPickup.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API AJumpBuffPickup : public APickupBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 4000.f;

	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 30.f;

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
