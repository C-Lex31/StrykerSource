// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"




// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	//PlayerCharacter = Cast<AStrykerCharacter>(GetOwner());
	// ...
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishing = true;
	ReplenishRate = ShieldAmount / ReplenishTime;
	AmountToReplenish += ShieldAmount;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (PlayerCharacter == nullptr) return;

	PlayerCharacter->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeeds,
		BuffTime
	);

	if (PlayerCharacter->GetCharacterMovement())
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}
void UBuffComponent::ResetSpeeds()
{
	if (PlayerCharacter == nullptr || PlayerCharacter->GetCharacterMovement() == nullptr) return;

	PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	PlayerCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}
void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (PlayerCharacter == nullptr) return;

	PlayerCharacter->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);
	if (PlayerCharacter->GetCharacterMovement())
	{
		PlayerCharacter->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}

	MulticastJumpBuff(BuffJumpVelocity);
}
void UBuffComponent::ResetJump()
{
	if (PlayerCharacter->GetCharacterMovement())
	{
		PlayerCharacter->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	}
	MulticastJumpBuff(InitialJumpVelocity);
}



// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || PlayerCharacter == nullptr || PlayerCharacter->GetIsEliminated()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	PlayerCharacter->SetHealth(FMath::Clamp(PlayerCharacter->GetHealth() + HealThisFrame, 0.f, PlayerCharacter->GetMaxHealth()));
	PlayerCharacter->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0.f || PlayerCharacter->GetHealth() >= PlayerCharacter->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ReplenishRampUp(float DeltaTime)
{
	if (!bReplenishing || PlayerCharacter == nullptr || PlayerCharacter->GetIsEliminated()) return;

	const float ReplenishThisFrame = ReplenishRate * DeltaTime;
	PlayerCharacter->SetShield(FMath::Clamp(PlayerCharacter->GetShield() + ReplenishThisFrame, 0.f, PlayerCharacter->GetMaxShield()));
	PlayerCharacter->UpdateHUDShield();
	AmountToReplenish -= ReplenishThisFrame;

	if (AmountToReplenish <= 0.f || PlayerCharacter->GetShield() >= PlayerCharacter->GetMaxShield())
	{
		bReplenishing = false;
		AmountToReplenish = 0.f;
	}
}


void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if(PlayerCharacter && PlayerCharacter->GetCharacterMovement())
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
	
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (PlayerCharacter && PlayerCharacter->GetCharacterMovement())
	{
		PlayerCharacter->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
	ReplenishRampUp(DeltaTime);
	// ...
}

