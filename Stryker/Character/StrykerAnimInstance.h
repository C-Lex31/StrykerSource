// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Stryker/Enumerations/TurnInPlace.h"
#include "StrykerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API UStrykerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(BlueprintReadOnly , Category =Character ,meta =(AllowPrivateAccess="true"))
	class AStrykerCharacter* StrykerCharacter;
	UPROPERTY(BlueprintReadOnly, Category =Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;
	UPROPERTY(BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquiped;
	UPROPERTY(BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;
	UPROPERTY(BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;
	UPROPERTY(BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandIKTransform;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float YawOffset;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Lean;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	class AWeaponBase* EquippedWeapon;
	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	ETurnInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bEliminated;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseFABRIK;
	
};
