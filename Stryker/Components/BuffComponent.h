// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STRYKER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()


	UPROPERTY()
	class AStrykerCharacter* PlayerCharacter;

	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	bool bReplenishing = false;
	float ReplenishRate = 0.f;
	float AmountToReplenish = 0.f;

	/**
* Speed buff
*/

	FTimerHandle SpeedBuffTimer;
	void ResetSpeeds();
	float InitialBaseSpeed=0.f;
	float InitialCrouchSpeed=0.f;

	/**
* Jump buff
*/
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;
public:	

	UBuffComponent();
	friend class AStrykerCharacter;

	void Heal(float HealAmount, float HealingTime);
	void ReplenishShield(float ShieldAmount, float ReplenishTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void BuffJump(float BuffJumpVelocity, float BuffTime);

	FORCEINLINE void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed) { InitialBaseSpeed = BaseSpeed; InitialCrouchSpeed = CrouchSpeed; }
	FORCEINLINE void SetInitialJumpZVelocity(float Velocity) { InitialJumpVelocity = Velocity; }

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:

	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);
	void ReplenishRampUp(float DeltaTime);
};
