// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableMovementComponent.h"

UThrowableMovementComponent::EHandleBlockingHitResult UThrowableMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{

	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return bTestFlag ? EHandleBlockingHitResult::AdvanceNextSubstep : EHandleBlockingHitResult::Deflect;

}

void UThrowableMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	if(!bTestFlag)
		Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}
