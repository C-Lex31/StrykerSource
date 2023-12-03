// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "Stryker/Character/StrykerCharacter.h "
#include "Stryker/Weapons/WeaponBase.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Stryker/PlayerController/StrykerPlayerController.h"
#include "Camera/CameraComponent.h"

void UWeaponComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->ZoomedFOV, DeltaTime, EquippedWeapon->ZoomInterpSpeed);
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (PlayerCharacter && PlayerCharacter->GetFollowCamera())
	{
		PlayerCharacter->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}


void UWeaponComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
}

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PlayerCharacter = Cast<AStrykerCharacter>(GetOwner());
	BaseWalkSpeed = 800;
	AimWalkSpeed = 400;
	// ...
}
// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	if (PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (PlayerCharacter->GetFollowCamera())
		{
			DefaultFOV = PlayerCharacter->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if (PlayerCharacter->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
	// ...

}
void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponComponent, EquippedWeapon);
	DOREPLIFETIME(UWeaponComponent, bIsAiming);
	DOREPLIFETIME_CONDITION(UWeaponComponent, CarriedAmmo, COND_OwnerOnly);
}
//CALLED ON SERVER from StrykerCharacter.cpp
void UWeaponComponent::EquipWeapon(AWeaponBase* WeaponToEqip)
{
	if (PlayerCharacter == nullptr || WeaponToEqip == nullptr) return;

	if (EquippedWeapon)
	{
		EquippedWeapon->DropWeapon();
	}
	EquippedWeapon = WeaponToEqip;
	EquippedWeapon->SetWeaponState(EWeaponState::Equipped);
	const USkeletalMeshSocket* HandSocket = (PlayerCharacter)->GetMesh()->GetSocketByName("RightHandSocket");
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, (Cast<ACharacter>(GetOwner()))->GetMesh());
	 }

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];

	EquippedWeapon->SetOwner(PlayerCharacter);
	EquippedWeapon->SetHUDAmmo();
	StrykerPlayerController = StrykerPlayerController == nullptr ? Cast<AStrykerPlayerController>(PlayerCharacter->Controller) : StrykerPlayerController;
	if (StrykerPlayerController)
		StrykerPlayerController->SetCarriedAmmo(CarriedAmmo);
	PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	PlayerCharacter->bUseControllerRotationYaw = true;
	
}

void UWeaponComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed )
	{
		Fire();
	}
}

void UWeaponComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		if (EquippedWeapon && PlayerCharacter)
		{
			CrosshairShootingFactor = 0.5f;
			
			if (!PlayerCharacter->HasAuthority()) LocalFire(HitTarget);
			 ServerFire(HitTarget);
			
		}
		StartFireTimer();
	}
}

void UWeaponComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || PlayerCharacter == nullptr) return;
	PlayerCharacter->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UWeaponComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UWeaponComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}

bool UWeaponComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;	
	return !EquippedWeapon->GetIsEmpty() || !bCanFire;
}


void UWeaponComponent::SetAiming(bool bAiming)
{
	bIsAiming = bAiming;
	ServerSetAiming(bAiming);
	if (PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	
	}
}

void UWeaponComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (PlayerCharacter)
	{
		PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UWeaponComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//Always Executed on Server
	MulticastFire(TraceHitTarget);
}

void UWeaponComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	
	if (PlayerCharacter  && PlayerCharacter->IsLocallyControlled() && !PlayerCharacter->HasAuthority()) return;
	LocalFire(TraceHitTarget);

}

void UWeaponComponent::OnRep_EquipWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::Equipped);
		const USkeletalMeshSocket* HandSocket = PlayerCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, PlayerCharacter->GetMesh());
		}

		PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		PlayerCharacter->bUseControllerRotationYaw = true;
		
		//EquippedWeapon->SetPlayerRef(PlayerCharacter);
	}
}

void UWeaponComponent::TraceCameraAim(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (PlayerCharacter)
		{
			float DistanceToCharacter = (PlayerCharacter->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UCrosshairInteractableInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}


void UWeaponComponent::PlayFireMontage(bool bAiming)
{
	if (this == nullptr || EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("AimFire") : FName("HipFire");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}



void UWeaponComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;
	if (PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}



// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	

	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		InterpFOV(DeltaTime);
	}
 

	// ...
}

