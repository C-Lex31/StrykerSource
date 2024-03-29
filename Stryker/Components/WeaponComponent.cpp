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
#include "Stryker/Weapons/Projectile.h"
#include "Animation/BlendSpace.h"
#include "Stryker/Weapons/Shotgun.h"
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
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Sniper, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
}

//CALLED ON SERVER ONLY
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

	EquippedWeapon->AddAmmo(ReloadAmount);
}

//On Clients Only
void UWeaponComponent::OnRep_CarriedAmmo()
{
	bool bJumpToShotgunEnd =
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;
	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}
//LC : To prevent double zoom in 
void UWeaponComponent::OnRep_Aiming()
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		bIsAiming = bAimButtonPressed;
	}
}

void UWeaponComponent::ShotgunShellReload()
{
	if(PlayerCharacter && PlayerCharacter->HasAuthority())
	UpdateShotgunAmmoValues();
}
//Server Only
void UWeaponComponent::UpdateShotgunAmmoValues()
{
	if (PlayerCharacter == nullptr || EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	StrykerPlayerController = StrykerPlayerController == nullptr ? Cast<AStrykerPlayerController>(PlayerCharacter->Controller) : StrykerPlayerController;
	if (StrykerPlayerController)
		StrykerPlayerController->SetCarriedAmmo(CarriedAmmo);

	EquippedWeapon->AddAmmo(1.f);
	bCanFire = true;
	if (EquippedWeapon->GetIsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
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

void UWeaponComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance )
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	//PlayerCharacter = Cast<AStrykerCharacter>(GetOwner());
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
	DOREPLIFETIME(UWeaponComponent, SecondaryWeapon);
	DOREPLIFETIME(UWeaponComponent, bIsAiming);
	DOREPLIFETIME_CONDITION(UWeaponComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UWeaponComponent, CombatState);
	DOREPLIFETIME(UWeaponComponent, Grenades);
}
//CALLED ON SERVER from StrykerCharacter.cpp
void UWeaponComponent::EquipWeapon(AWeaponBase* WeaponToEquip)
{
	if (PlayerCharacter == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;


	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
	PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	PlayerCharacter->bUseControllerRotationYaw = true;
}
//called on server only
void UWeaponComponent::SwapWeapons()
{
	if (CombatState != ECombatState::ECS_Unoccupied || PlayerCharacter == nullptr || !PlayerCharacter->HasAuthority())return;

	//bCanFire = true;
	PlayerCharacter->PlayEquipWeaponMontage();
	CombatState = ECombatState::ECS_EquipWeapon;
	PlayerCharacter->SetFinishedSwapping(false);


}
void UWeaponComponent::EquipPrimaryWeapon(AWeaponBase* WeaponToEquip)
{
	if (WeaponToEquip == nullptr)return;
	if (EquippedWeapon)
		EquippedWeapon->DropWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_EquippedPrimary);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(PlayerCharacter);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	CombatState = ECombatState::ECS_Unoccupied;
	if (EquippedWeapon->GetIsEmpty())
		Reload();


	PlayerCharacter->SetCrosshair();
	PlayEquipWeaponSound(EquippedWeapon);
}

void UWeaponComponent::EquipSecondaryWeapon(AWeaponBase* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);
	SecondaryWeapon->SetOwner(PlayerCharacter);
}





void UWeaponComponent::OnRep_EquipWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_EquippedPrimary);
		AttachActorToRightHand(EquippedWeapon);
		PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		PlayerCharacter->bUseControllerRotationYaw = true;
		PlayerCharacter->SetCrosshair();
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->EnableCustomDepth(false);
	}
}

void UWeaponComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && PlayerCharacter)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBackpack(SecondaryWeapon);
		PlayEquipWeaponSound(EquippedWeapon);
	}
}

void UWeaponComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (PlayerCharacter == nullptr || PlayerCharacter->GetMesh() == nullptr || EquippedWeapon == nullptr || ActorToAttach==nullptr) return;
	const USkeletalMeshSocket* HandSocket = (PlayerCharacter)->GetMesh()->GetSocketByName(EquippedWeapon->GetRightHandSocket());
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, (Cast<ACharacter>(GetOwner()))->GetMesh());
	}
}

void UWeaponComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (PlayerCharacter == nullptr || PlayerCharacter->GetMesh() == nullptr || EquippedWeapon == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = (PlayerCharacter)->GetMesh()->GetSocketByName(EquippedWeapon->GetLeftHandSocket());
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, (Cast<ACharacter>(GetOwner()))->GetMesh());
	}
}

void UWeaponComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (PlayerCharacter == nullptr || PlayerCharacter->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HolsterSocket = PlayerCharacter->GetMesh()->GetSocketByName(SecondaryWeapon->GetHolsterSocket());
	if (HolsterSocket)
	{
		HolsterSocket->AttachActor(ActorToAttach, PlayerCharacter->GetMesh());
	}
}

void UWeaponComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed )
	{
		Fire();
	}
}

bool UWeaponComponent::GetShouldSwapWeapons()
{
	return EquippedWeapon != nullptr && SecondaryWeapon != nullptr && !bIsAiming && !bFireButtonPressed && !bLocallyReloading ;
}

void UWeaponComponent::Fire()
{

	if (CanFire())
	{
		bCanFire = false;
		if (EquippedWeapon && PlayerCharacter)
		{
			CrosshairShootingFactor = 0.5f;
			
			 switch (EquippedWeapon->FireType)
			 {
			 case EFireType::EFT_Projectile:
				 FireProjectileWeapon();
				 break;
			 case EFireType::EFT_HitScan:
				 FireHitScanWeapon();
				 break;
			 case EFireType::EFT_Shotgun:
				 FireShotgun();
				 break;
			 }
		}
	//	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled() )
			StartFireTimer();
	}
	else if (EquippedWeapon && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon->GetIsEmpty() )
	{
		
		Reload();
	}
}
void UWeaponComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && PlayerCharacter)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!PlayerCharacter->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget);
	}
}

void UWeaponComponent::FireHitScanWeapon()
{
	if (EquippedWeapon && PlayerCharacter)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!PlayerCharacter->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget);
	}
}

void UWeaponComponent::FireShotgun()
{

	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun)
	{
		TArray<FVector_NetQuantize>HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (!PlayerCharacter->HasAuthority()) ShotgunLocalFire(HitTargets);
		ServerShotgunFire(HitTargets);
	}
}

void UWeaponComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->GetIsFull() && !bLocallyReloading)
	{

		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}

void UWeaponComponent::TossGrenade()
{
	if (Grenades == 0.f)return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;
	//CombatState = ECombatState::ECS_TossingGrenade;

	if (PlayerCharacter)
	{
		TossGrenadeCosmetic();
	}

	if (PlayerCharacter)//&& !PlayerCharacter->HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, "Server- LC");
		ServerTossGrenadeCosmetic();
	}
	/*
	if (PlayerCharacter && !PlayerCharacter->HasAuthority())
	{
		ServerTossGrenadeCosmetic();
	}
	if (PlayerCharacter && PlayerCharacter->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
	*/
}
void UWeaponComponent::TossGrenadeCosmetic()
{
	if (PlayerCharacter )
	{
		PlayerCharacter->PlayTossGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
}
void UWeaponComponent::ServerTossGrenadeCosmetic_Implementation()
{
	if (Grenades == 0.f)return;
	CombatState = ECombatState::ECS_TossingGrenade;
	//Could definitely use a multicast RPC here instead of Rep Notify
	
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	
	UpdateHUDGrenades();
	if(PlayerCharacter && !PlayerCharacter->IsLocallyControlled())
	 TossGrenadeCosmetic();

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

	bLocallyReloading = false;
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

void UWeaponComponent::FinishTossGrenade()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

//Called LOCALLY from anim notify in blueprints
void UWeaponComponent::BP_TossGrenade()
{
	ShowAttachedGrenade(false);
	//if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	//{
		//ServerTossGrenade(HitTarget);
	//}
	//if (PlayerCharacter->HasAuthority())
		//ServerTossGrenade(HitTarget);

	if (!PlayerCharacter->HasAuthority() && PlayerCharacter->IsLocallyControlled()) 
		LocalToss(HitTarget);
	ServerTossGrenade(HitTarget);

	//Toss();
	/*if (EquippedWeapon && PlayerCharacter)
	{
		if (!PlayerCharacter->HasAuthority()&&PlayerCharacter->IsLocallyControlled()) Toss();
		ServerTossGrenade(HitTarget);
	}*/

}
void UWeaponComponent::BP_FinishEquip()
{
	//if (PlayerCharacter && PlayerCharacter->HasAuthority())
	//CombatState = ECombatState::ECS_Unoccupied;
	ServerFinishSwappingWeapon();
	//if (EquippedWeapon->GetIsEmpty())
		//Reload();
	if (PlayerCharacter)
		PlayerCharacter->SetFinishedSwapping(true);
	//if(PlayerCharacter && PlayerCharacter->HasAuthority())
	//ServerFinishSwappingWeapon();
}
void UWeaponComponent::BP_EquipAttachWeapon()
{
	ServerEquipAttachWeapon();
	AWeaponBase* temp = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = temp;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_EquippedPrimary);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(PlayerCharacter);
	EquippedWeapon->SetHUDAmmo();
	
	//UpdateCarriedAmmo();
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);

	
}
void UWeaponComponent::ServerEquipAttachWeapon_Implementation()
{
	AWeaponBase* temp = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = temp;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_EquippedPrimary);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(PlayerCharacter);
	EquippedWeapon->SetHUDAmmo();

	UpdateCarriedAmmo();
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);

//	if (EquippedWeapon->GetIsEmpty())
		//Reload();
}
void UWeaponComponent::ServerFinishSwappingWeapon_Implementation()
{
	CombatState = ECombatState::ECS_Unoccupied;
	//if (EquippedWeapon->GetIsEmpty())
		//Reload();
		
}
void UWeaponComponent::ServerTossGrenade_Implementation(const FVector_NetQuantize& Target)
{
	MulticastToss(Target);
}

void UWeaponComponent::MulticastToss_Implementation(const FVector_NetQuantize& Target)
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled() && !PlayerCharacter->HasAuthority()) return;
	LocalToss(Target);
}
void UWeaponComponent::LocalToss(const FVector_NetQuantize& Target)
{
	if (PlayerCharacter && SSR_GrenadeClass && PlayerCharacter->GetAttachedGrenade())
	{
		
		const FVector StartingLocation = PlayerCharacter->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = PlayerCharacter;
		SpawnParams.Instigator = PlayerCharacter;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				SSR_GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
			);
		}

		/*if (!PlayerCharacter)return;
		//if (EquippedWeapon == nullptr) return;
		//APawn* InstigatorPawn = Cast<APawn>(GetOwner());
		UWorld* World = GetWorld();
		AProjectile* SpawnedProjectile = nullptr;
		const FVector StartingLocation = PlayerCharacter->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = HitTarget - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = PlayerCharacter;
		SpawnParams.Instigator = PlayerCharacter;
		if (PlayerCharacter->HasAuthority()) // On server 
		{
			if (PlayerCharacter->IsLocallyControlled())
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(GrenadeClass, StartingLocation, ToTarget.Rotation(), SpawnParams);
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Server- LC");
				SpawnedProjectile->bUseServerSideRewind = false;
				//SpawnedProjectile->SetProjectileDamage(Damage); //set projectile damage equal to projectile weapon damage
			}
			else // server, not locally controlled - spawn non-replicated projectile, no SSR
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(SSR_GrenadeClass, StartingLocation, ToTarget.Rotation(), SpawnParams);
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "Server-Not LC");
				SpawnedProjectile->bUseServerSideRewind = false;
			}
		}

		else
		{
			if (PlayerCharacter->IsLocallyControlled()) // client, locally controlled - spawn non-replicated projectile, use SSR
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(SSR_GrenadeClass, StartingLocation, ToTarget.Rotation(), SpawnParams);
				if (SpawnedProjectile)
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "Client- LC");
					SpawnedProjectile->bUseServerSideRewind = true;
					//	SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					//SpawnedProjectile->SetProjectileDamage(Damage);
				}

			}
			else // client, not locally controlled - spawn non-replicated projectile, no SSR
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(SSR_GrenadeClass, StartingLocation, ToTarget.Rotation(), SpawnParams);
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Emerald, "CLIENT-Not LC");
				//	SpawnedProjectile->bUseServerSideRewind = false;
			}
		}*/

	}
}


void UWeaponComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || PlayerCharacter == nullptr || !(CombatState==ECombatState::ECS_Unoccupied)) return;
	PlayerCharacter->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UWeaponComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UWeaponComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr ) return;
	bCanFire = true;
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, "FIRETIMERFINISHED");
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	else
	{
		UKismetSystemLibrary::K2_ClearTimerHandle(GetWorld(), FireTimer);
	}
	if (EquippedWeapon->GetIsEmpty() && CombatState==ECombatState::ECS_Unoccupied)
	{
	//	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, "FIRETIMERFINISHED");
		//Reload();
	}
}

bool UWeaponComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	
	if (!EquippedWeapon->GetIsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString(TEXT("%s"), bCanFire));
	if (bLocallyReloading) return false;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString(TEXT("%s"), bCanFire));
	if (!(CombatState == ECombatState::ECS_Unoccupied))return false;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, FString(TEXT("%s"), bCanFire));
	if (EquippedWeapon->GetIsEmpty()) return false;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString(TEXT("%s"), bCanFire));
	if (!bCanFire) return false;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White, FString(TEXT("%s"), bCanFire));

	return true;
	//return !EquippedWeapon->GetIsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;

}

void UWeaponComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		if (PlayerCharacter && !PlayerCharacter->IsLocallyControlled())
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
			Fire();
		break;
	case ECombatState::ECS_TossingGrenade:
		if (PlayerCharacter && !PlayerCharacter->IsLocallyControlled())
			TossGrenadeCosmetic();
		break;

	case ECombatState::ECS_EquipWeapon:
		if (PlayerCharacter && !PlayerCharacter->IsLocallyControlled())
			PlayerCharacter->PlayEquipWeaponMontage();
		break;
	
	}
}


void UWeaponComponent::SetAiming(bool bAiming)
{
	if (PlayerCharacter == nullptr || EquippedWeapon == nullptr)return;

	bIsAiming = bAiming;
	ServerSetAiming(bAiming);
	if (PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	
	}
	if (PlayerCharacter->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)
	{
		PlayerCharacter->ShowSniperScopeWidget(bAiming);
	}
	if (PlayerCharacter->IsLocallyControlled()) bAimButtonPressed = bIsAiming;
}



void UWeaponComponent::HandleReload()
{
	if(PlayerCharacter)
		PlayerCharacter->PlayReloadMontage();
}

void UWeaponComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//Always Executed on Server
	MulticastFire(TraceHitTarget);
}

void UWeaponComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	
	if (PlayerCharacter  && PlayerCharacter->IsLocallyControlled() && !PlayerCharacter->HasAuthority()) return; // Not playing Fire for Player who fired weapon

	LocalFire(TraceHitTarget);

}
void UWeaponComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UWeaponComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled() && !PlayerCharacter->HasAuthority()) return;
	ShotgunLocalFire(TraceHitTargets);
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

void UWeaponComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (!Shotgun || !PlayerCharacter) return;
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		PlayerCharacter->PlayFireMontage(bIsAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}

}
void UWeaponComponent::ServerReload_Implementation()
{
	if (PlayerCharacter == nullptr || EquippedWeapon == nullptr)return;

	CombatState = ECombatState::ECS_Reloading;	//Could definitely use a multicast RPC here instead of Rep Notify
	//To ensure authority player can see the reload animations of simulated proxies on his machine
	if(!PlayerCharacter->IsLocallyControlled())
		HandleReload();
	
}


void UWeaponComponent::UpdateHUDGrenades()
{

	StrykerPlayerController = StrykerPlayerController == nullptr ? Cast<AStrykerPlayerController>(PlayerCharacter->Controller) : StrykerPlayerController;
	if (StrykerPlayerController)
		StrykerPlayerController->SetGrenadeAmmo(Grenades);//Client RPC
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

void UWeaponComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (PlayerCharacter && PlayerCharacter->GetAttachedGrenade())
	{
		PlayerCharacter->GetAttachedGrenade()->SetVisibility(bShowGrenade);
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


