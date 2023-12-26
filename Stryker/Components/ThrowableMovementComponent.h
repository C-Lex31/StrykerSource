// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ThrowableMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API UThrowableMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

	bool bTestFlag = false;
protected:
	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
public:
	FORCEINLINE void SetFlagValue(bool bValue) { bTestFlag = bValue; }
};
