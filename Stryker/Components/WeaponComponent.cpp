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
#include "Sound/SoundCue.h"
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
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRLAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo);
}

void UWeaponComponent::UpdateAmmoValues()
{
	if (PlayerCharacter == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	StrykerPlayerController = StrykerPlayerController == nullptr ? Cast<AStrykerPlayerController>(PlayerCharacter->Controller) : StrykerPlayerController;
	if (StrykerPlayerController)
		StrykerPlayerController->SetCarriedAmmo(CarriedAmmo);

	EquippedWeapon->AddAmmo(-ReloadAmount);
}

int32 UWeaponComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
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
	DOREPLIFETIME(UWeaponComponent, CombatState);
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

	if (EquippedWeapon->GetIsEmpty())
	{
		Reload();
	}

	PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	PlayerCharacter->bUseControllerRotationYaw = true;
	PlayEquipWeaponSound(EquippedWeapon);
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

void UWeaponComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied)
		ServerReload();
}

void UWeaponComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
	if (EquippedWeapon && EquippedWeapon->GetIsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UWeaponComponent::FinishReloading()
{
	if (PlayerCharacter == nullptr) return;

	if (PlayerCharacter->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bFireButtonPressed)
	{
		Fire();
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
	if (EquippedWeapon->GetIsEmpty())
	{
		Reload();
	}
}

bool UWeaponComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;	
	return !EquippedWeapon->GetIsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UWeaponComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
			Fire();
	
	}
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
	if (PlayerCharacter && CombatState == ECombatState::ECS_Unoccupied)
	{
		PlayerCharacter->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UWeaponComponent::HandleReload()
{
	PlayerCharacter->PlayReloadMontage();
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

void UWeaponComponent::ServerReload_Implementation()
{
	if (PlayerCharacter == nullptr)return;

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
	
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
		PlayEquipWeaponSound(EquippedWeapon);
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

void UWeaponComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	StrykerPlayerController = StrykerPlayerController == nullptr ? Cast<AStrykerPlayerController>(PlayerCharacter->Controller) : StrykerPlayerController;
	if (StrykerPlayerController)
		StrykerPlayerController->SetCarriedAmmo(CarriedAmmo);
}

void UWeaponComponent::PlayEquipWeaponSound(AWeaponBase* WeaponToEquip)
{
	if (PlayerCharacter && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			PlayerCharacter->GetActorLocation()
		);
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

