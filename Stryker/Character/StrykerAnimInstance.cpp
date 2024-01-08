// Fill out your copyright notice in the Description page of Project Settings.


#include "StrykerAnimInstance.h"
#include "StrykerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Stryker/Weapons/WeaponBase.h "
#include "Stryker/Enumerations/CombatState.h"
void UStrykerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	StrykerCharacter = Cast<AStrykerCharacter>(TryGetPawnOwner());

}

void UStrykerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (StrykerCharacter == nullptr)
	{
		StrykerCharacter = Cast<AStrykerCharacter>(TryGetPawnOwner());
	}
	if (StrykerCharacter == nullptr) return;

	FVector Velocity = StrykerCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();

	bIsInAir = StrykerCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = StrykerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f ? true : false;
	bWeaponEquiped = StrykerCharacter->IsWeaponEquiped();
	bIsCrouched = StrykerCharacter->bIsCrouched;
	bIsAiming = StrykerCharacter->IsAiming();
	TurningInPlace = StrykerCharacter->GetTurningInPlace();
	EquippedWeapon = StrykerCharacter->GetEquippedWeapon();
	bEliminated = StrykerCharacter->GetIsEliminated();

	bUseFABRIK = StrykerCharacter->GetCombatState() == ECombatState::ECS_Unoccupied; 
	if (StrykerCharacter->IsLocallyControlled() && StrykerCharacter->GetCombatState() != ECombatState::ECS_TossingGrenade)
	{
		bUseFABRIK = !StrykerCharacter->GetIsLocallyReloading();
	}
	// Offset Yaw for Strafing
	FRotator AimRotation = StrykerCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(StrykerCharacter->GetVelocity());
	//YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
	CharacterRotationLastFrame = CharacterRotation;
	YawOffset = DeltaRotation.Yaw;
	CharacterRotation = StrykerCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	AO_Yaw = StrykerCharacter->GetAO_Yaw();
	AO_Pitch =StrykerCharacter->GetAO_Pitch();

	if (bWeaponEquiped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && StrykerCharacter->GetMesh())
	{
		LeftHandIKTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		StrykerCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandIKTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandIKTransform.SetLocation(OutPosition);
		LeftHandIKTransform.SetRotation(FQuat(OutRotation));


	if (StrykerCharacter->IsLocallyControlled())
	{
			bLocallyControlled = true;
			FTransform RightHandTransform = StrykerCharacter->GetMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - StrykerCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 20.f);
	}

	}
}
