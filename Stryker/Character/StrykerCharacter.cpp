

#include "StrykerCharacter.h"
#include "Stryker/Weapons/WeaponBase.h"
#include "Stryker/Components/WeaponComponent.h"
#include "Stryker/Components/BuffComponent.h"
#include "Stryker/Components/LagCompensationComponent.h"
#include "Stryker/Stryker.h"
#include "StrykerAnimInstance.h"
#include "Stryker/PlayerController/StrykerPlayerController.h"
#include "Stryker/StrykerGameMode.h"
#include "Stryker/UI/HUD/Crosshair.h"
#include "Stryker/UI/HUD/CharacterOverlay.h"
#include "Stryker/PlayerState/StrykerPlayerState.h"
#include "Stryker/Enumerations/WeaponTypes.h"

#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/BoxComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h "
#include "Kismet/GameplayStatics.h"

#include "Blueprint/WidgetLayoutLibrary.h"

#include "Engine/SkeletalMeshSocket.h"

#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"


//////////////////////////////////////////////////////////////////////////
// AStrykerCharacter


void AStrykerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps((OutLifetimeProps));
	DOREPLIFETIME_CONDITION(AStrykerCharacter, OverlappedWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AStrykerCharacter, Health);
	DOREPLIFETIME(AStrykerCharacter, Shield);
}



AStrykerCharacter::AStrykerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	this->Tags.Add("Player");
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	//GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	//CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	WeaponComponent->SetIsReplicated(true);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	TurningInPlace = ETurnInPlace::NotTurning;

	TL_Dissolve = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LCC = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));
#pragma region HitboxesForSSR
	/**
* Hit boxes for server-side rewind
*/
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);


	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);


	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
#pragma endregion

	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}
void AStrykerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	PC = Cast<AStrykerPlayerController>(NewController) ;
	if (PC)
	{

		PC->InitializeGameHUD();
		InitializeCrosshair();
		
		PC->SetHealth(Health, MaxHealth);
		PC->SetShield(Shield, MaxShield);
		if (WeaponComponent)
		{
			PC->SetGrenadeAmmo(WeaponComponent->GetGrenadeCount());
			
		}

	}
	PS = GetPlayerState<AStrykerPlayerState>();
	if (PS)
	{
		PS->AddToScore(0.f);
		PS->AddToDeaths(0.f);
	}
	
	
	//Cast<AStrykerPlayerController>(Controller)->SetGrenadeAmmo(WeaponComponent->GetGrenadeCount());
		OnTakeAnyDamage.AddDynamic(this, &AStrykerCharacter::ReceiveDamage);
}

void AStrykerCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	LocalPC = Cast<AStrykerPlayerController>(Controller);
	//Add Input Mapping Context
	if (const ULocalPlayer* Player = (GEngine && GetWorld()) ? GEngine->GetFirstGamePlayer(GetWorld()) : nullptr)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Player))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}

	if (bSpawnWithDefaultWeapon && LocalPC)
	{
		SpawnDefaultWeapon();
		if (WeaponComponent->EquippedWeapon)
			LocalPC->SetWeaponAmmo(WeaponComponent->EquippedWeapon->GetAmmo());
		LocalPC->SetCarriedAmmo(WeaponComponent->CarriedAmmo);
	}
}

void AStrykerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AimOffset(DeltaSeconds);
	OccludeCharacter();

	if (this && IsLocallyControlled())
	{
		UpdateCrosshair(DeltaSeconds);
		CameraTraceEndLocation = TraceCameraAim();
		CrosshairLogicUpdate();
	}


}



#pragma region TickMethods



FVector AStrykerCharacter::TraceCameraAim()
{
	if (!Crosshair) return FVector{ 0 };

	FHitResult TraceHitResult;
	FVector Start = FollowCamera->K2_GetComponentLocation();

	float DistanceToCharacter = (GetActorLocation() - Start).Size();
	Start += FollowCamera->GetForwardVector() * (DistanceToCharacter + 100.f); //Adding an offset to trace start to avoid colliding with player
	FVector End = FollowCamera->K2_GetComponentLocation() + FollowCamera->GetForwardVector() * 15000.f;
	TArray<AActor*>ActorsToIgnore;
	ActorsToIgnore.Add((WeaponComponent->EquippedWeapon));
	
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), Start, End, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, TraceHitResult, true, FColor::Blue, FColor::Green, 0.05f);

	if (TraceHitResult.bBlockingHit)
	{
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UCrosshairInteractableInterface>())
			Crosshair->ColorAndOpacity = FLinearColor::Red;
		else
			Crosshair->ColorAndOpacity = FLinearColor::White;
		return TraceHitResult.Location;
	}
	else
	{
		Crosshair->ColorAndOpacity = FLinearColor::White;
		return TraceHitResult.TraceEnd;
	}
}

void AStrykerCharacter::OccludeCharacter()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold || (WeaponComponent && WeaponComponent->EquippedWeapon && WeaponComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper &&WeaponComponent->bIsAiming ))
	{
		GetMesh()->SetVisibility(false);
		if (WeaponComponent&& WeaponComponent->EquippedWeapon && WeaponComponent->EquippedWeapon->GetWeaponMesh())
		{
			WeaponComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (WeaponComponent && WeaponComponent->EquippedWeapon && WeaponComponent->EquippedWeapon->GetWeaponMesh())
		{
			WeaponComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AStrykerCharacter::UpdateCrosshair(float DeltaTime)
{

	// Calculate crosshair spread

			// [0, 600] -> [0, 1]
	if (!WeaponComponent || !Crosshair)return;
	FVector2D WalkSpeedRange(0.f, GetCharacterMovement()->MaxWalkSpeed);
	FVector2D VelocityMultiplierRange(0.f, 0.2f);
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;

	if (!WeaponComponent->bIsAiming)
		CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
	if (GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.4f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}
	if (WeaponComponent->bIsAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.3f, DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("%f"),WeaponComponent->CrosshairShootingFactor));
	WeaponComponent->CrosshairShootingFactor = FMath::FInterpTo(WeaponComponent->CrosshairShootingFactor, 0.f, DeltaTime, 40.f);
	CrosshairSpreadScale = //0.5f +
		CrosshairVelocityFactor +
		CrosshairInAirFactor -
		CrosshairAimFactor +
		WeaponComponent->CrosshairShootingFactor;
	if (Crosshair)
		Crosshair->UpdateCrosshairPosition(CrosshairSpreadScale);
}

void AStrykerCharacter::CrosshairLogicUpdate()
{
	if (!WeaponComponent->EquippedWeapon || !Crosshair) return;

	const USkeletalMeshSocket* MuzzleSocket = WeaponComponent->EquippedWeapon->GetWeaponMesh()->GetSocketByName("Muzzle");
	FHitResult HitResult;
	FVector2D ObstacleCrosshairLocation;
	
	TArray<AActor*>ActorsToIgnore;

	ActorsToIgnore.Add((WeaponComponent->EquippedWeapon));
	if (MuzzleSocket)
	{
		FVector TraceStart = (MuzzleSocket->GetSocketTransform(WeaponComponent->EquippedWeapon->GetWeaponMesh())).GetLocation();
		FRotator TraceRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, CameraTraceEndLocation);

		FVector TraceEnd = TraceStart + UKismetMathLibrary::GetForwardVector(TraceRotation) * (WeaponComponent->bIsAiming ? CrossObstacleTraceLength / 2.f : CrossObstacleTraceLength);
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, TraceEnd, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, true, FColor::Red, FColor::Green, 0.05f);
		CrosshairLocation = HitResult.bBlockingHit ? HitResult.Location : CameraTraceEndLocation;
		bCrosshairHasObstacle = UKismetMathLibrary::NotEqual_VectorVector(CameraTraceEndLocation, CrosshairLocation, 1.f);
		WeaponComponent->SetHitTarget(bCrosshairHasObstacle ? HitResult.ImpactPoint : CameraTraceEndLocation);
		if (bCrosshairHasObstacle)
		{
			if (LocalPC && LocalPC->GetGameHUD() && LocalPC->GetGameHUD()->ObstacleCrosshair)
			{

				
				UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(LocalPC, CrosshairLocation, ObstacleCrosshairLocation, false);
				LocalPC->GetGameHUD()->ObstacleCrosshair->SetRenderOpacity(1.f);
				Crosshair->SetRenderOpacity(0.2f);
				UWidgetLayoutLibrary::SlotAsCanvasSlot(LocalPC->GetGameHUD()->ObstacleCrosshair)->SetPosition(ObstacleCrosshairLocation);
			}
		}
		else
		{
			if (LocalPC && LocalPC->GetGameHUD() && LocalPC->GetGameHUD()->ObstacleCrosshair)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "TYEFRGDCSXTRYEDCSXTRECDSX");
				LocalPC->GetGameHUD()->ObstacleCrosshair->SetRenderOpacity(0.f);
				Crosshair->SetRenderOpacity(1.f);
			}
		}

	}
}

void AStrykerCharacter::AimOffset(float DeltaTime)
{
	if (WeaponComponent && WeaponComponent->EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurnInPlace::NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		//bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		//bUseControllerRotationYaw = false;
		TurningInPlace = ETurnInPlace::NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AStrykerCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 45.f)
	{
		TurningInPlace = ETurnInPlace::Right;
	}
	else if (AO_Yaw < -45.f)
	{
		TurningInPlace = ETurnInPlace::Left;
	}
	else
	{
		//TurningInPlace = ETurnInPlace::NotTurning;
	}
	if (TurningInPlace != ETurnInPlace::NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurnInPlace::NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

#pragma endregion TickMethods



#pragma region Health_Dmg

void AStrykerCharacter::OnRep_Health(float LastHealth)
{

	UpdateHUDHealth();
	if (Health<LastHealth && GetCombatState() == ECombatState::ECS_Unoccupied)
		PlayHitReactMontage();
}

void AStrykerCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield && GetCombatState() == ECombatState::ECS_Unoccupied)
		PlayHitReactMontage();
}

//Called on server only
void AStrykerCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bEliminated) return;

	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			Shield = 0.f;
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
		}
	}
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);

	UpdateHUDHealth();
	UpdateHUDShield();
	if(GetCombatState() == ECombatState::ECS_Unoccupied)
		PlayHitReactMontage();
	if (Health==0.f)
    {
	AStrykerGameMode* GM = GetWorld()->GetAuthGameMode<AStrykerGameMode>();
		if (GM)
		{
			PC = PC == nullptr ? Cast<AStrykerPlayerController>(Controller) : PC;
			AStrykerPlayerController* AttackerController = Cast<AStrykerPlayerController>(InstigatedBy);
			GM->PlayerEliminated(this, PC, AttackerController);
		}
    }
}
void AStrykerCharacter::PlayHitReactMontage()
{

	if (WeaponComponent == nullptr || WeaponComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AStrykerCharacter::PlayFireMontage(bool bAiming)
{
	if (this == nullptr || WeaponComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("AimFire") : FName("HipFire");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AStrykerCharacter::PlayTossGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && TossGrenadeMontage)
	{
		AnimInstance->Montage_Play(TossGrenadeMontage);
	}
}
void AStrykerCharacter::PlayEquipWeaponMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && WeaponEquipMontage)
	{
		AnimInstance->Montage_Play(WeaponEquipMontage);
	}
}
void AStrykerCharacter::PlayReloadMontage()
{
	if (WeaponComponent == nullptr || WeaponComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (WeaponComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("AR");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("AR");
			break;
		case EWeaponType::EWT_SMG:
			SectionName = FName("AR");
			break;
		case EWeaponType::EWT_Sniper:
			SectionName = FName("AR");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		default:
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AStrykerCharacter::UpdateHUDHealth()
{
	PC = (PC == nullptr )? Cast<AStrykerPlayerController>(Controller) : PC;
	if (PC)
	{
		PC->SetHealth(Health, MaxHealth);
	}
}

void AStrykerCharacter::UpdateHUDShield()
{
	PC = (PC == nullptr) ? Cast<AStrykerPlayerController>(Controller) : PC;
	if (PC)
	{
		PC->SetShield(Shield, MaxShield);
	}
}

void AStrykerCharacter::SpawnDefaultWeapon()
{
	AStrykerGameMode* StrykerGameMode = Cast<AStrykerGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (StrykerGameMode && World && !bEliminated && DefaultWeaponClass)
	{
		AWeaponBase* StartingWeapon = World->SpawnActor<AWeaponBase>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (WeaponComponent)
		{
			WeaponComponent->EquipWeapon(StartingWeapon);
		}
	}
}

#pragma endregion Health_Dmg

#pragma region Elimination
//Called from game mode and game mode exists on server only
void AStrykerCharacter::ServerEliminated()
{
	DropOrDestroyWeapons();
//	Client_Eliminated();
	MulticastEliminated();
	GetWorldTimerManager().SetTimer(
		TH_EliminationTimer,
		this,
		&AStrykerCharacter::OnElimTimerFinished,
		RespawnDelay
	);
}
void AStrykerCharacter::Client_Eliminated_Implementation()
{

	if (Crosshair)
	{
		Crosshair->RemoveFromParent();
		Crosshair = nullptr;
	}
	UKismetSystemLibrary::K2_ClearTimerHandle(GetWorld(), TH_CenterCrosshair);
	if (LocalPC && LocalPC->GetGameHUD())
	{
		LocalPC->GetGameHUD()->RemoveFromParent();
		LocalPC->SetGameHUD(nullptr);
	}

}
void AStrykerCharacter::MulticastEliminated_Implementation()
{
	bEliminated = true;
	if (Crosshair && IsLocallyControlled())
	{
		Crosshair->RemoveFromParent();
		Crosshair = nullptr;
	}
	UKismetSystemLibrary::K2_ClearTimerHandle(GetWorld(), TH_CenterCrosshair);
	if (LocalPC && LocalPC->GetGameHUD() && IsLocallyControlled())
	{
		LocalPC->GetGameHUD()->RemoveFromParent();
		LocalPC->SetGameHUD(nullptr);
	}

	PlayEliminationMontage();
	if (DissolveMaterial)
	{
		DynamicDissolveMaterial = UMaterialInstanceDynamic::Create(DissolveMaterial, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterial);
		//DynamicDissolveMaterial->SetScalarParameterValue(TEXT("Dissolve"), 0.f);
		//DynamicDissolveMaterial->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (PC)
	{
		DisableInput(PC);
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Spawn elim bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
	bool bHideSniperScope = IsLocallyControlled() &&
		WeaponComponent &&
		WeaponComponent->bIsAiming &&
		WeaponComponent->EquippedWeapon &&
		WeaponComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
}
void AStrykerCharacter::DropOrDestroyWeapon(AWeaponBase* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->DropWeapon();
	}
}
void AStrykerCharacter::DropOrDestroyWeapons()
{
	if (WeaponComponent)
	{
		if (WeaponComponent->EquippedWeapon)
		{
			DropOrDestroyWeapon(WeaponComponent->EquippedWeapon);
		}
		if (WeaponComponent->SecondaryWeapon)
		{
			DropOrDestroyWeapon(WeaponComponent->SecondaryWeapon);
		}
	}
}
void AStrykerCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}

void AStrykerCharacter::PlayEliminationMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EliminationMontage)
	{
		AnimInstance->Montage_Play(EliminationMontage);
	}
}

void AStrykerCharacter::OnElimTimerFinished()
{
	AStrykerGameMode* StrykerGameMode = GetWorld()->GetAuthGameMode<AStrykerGameMode>();
	if (StrykerGameMode)
	{
		StrykerGameMode->RequestRespawn(this, Controller);
	}
}
void AStrykerCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AStrykerCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && TL_Dissolve)
	{
		TL_Dissolve->AddInterpFloat(DissolveCurve, DissolveTrack);//Add curve to track on timeline
		TL_Dissolve->Play();
	}
}

void AStrykerCharacter::UpdateDissolveMaterial(float DissolveAmount)
{
	if (DynamicDissolveMaterial)
	{
		DynamicDissolveMaterial->SetScalarParameterValue(TEXT("Dissolve"), DissolveAmount);
	}
}
#pragma endregion Elimination


#pragma region InitializationMethods


void AStrykerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AStrykerCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AStrykerCharacter::Look);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AStrykerCharacter::EventInteract);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AStrykerCharacter::EventCrouch);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AStrykerCharacter::EventAimStart);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AStrykerCharacter::EventAimEnd);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AStrykerCharacter::EventFireStart);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AStrykerCharacter::EventFireStop);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AStrykerCharacter::EventReload);
		EnhancedInputComponent->BindAction(TossGrenadeAction, ETriggerEvent::Started, this, &AStrykerCharacter::EventTossGrenade);
	}
}
void AStrykerCharacter::InitializeCrosshair_Implementation()
{
	Crosshair = CreateWidget<UCrosshair>( UGameplayStatics::GetPlayerController(GetWorld() , 0) ,CrosshairClass);
	if (Crosshair)
	{
		Crosshair->StoreInitialCrosshairSegmentPos();
		Crosshair->AddToViewport();
		GetWorld()->GetTimerManager().SetTimer(
			TH_CenterCrosshair,
			this,
			&AStrykerCharacter::CenterCrosshair,
			0.05f,
			true
		);
	}
}
void AStrykerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (WeaponComponent)
	{
		WeaponComponent->PlayerCharacter = this;
	
			
	}
	if (BuffComponent)
	{
		BuffComponent->PlayerCharacter = this;
		BuffComponent->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpZVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LCC)
	{
		LCC->PlayerCharacter = this;
	}
}

#pragma endregion InitializationMethods


#pragma region CustomTickMethods

void AStrykerCharacter::CenterCrosshair()
{
	FVector2D ViewportSize;
	//ViewportSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	if (Crosshair)
		Crosshair->SetPositionInViewport(FVector2D{ ViewportSize.X / 2.f , ViewportSize.Y / 2.f });

}

#pragma endregion CustomTickMethods


#pragma region Interaction

//Not called on server . Replication works one-way i.e. Server to Client 
void AStrykerCharacter::OnRep_OverlappedWeapon(AWeaponBase* LastWeapon)
{
	if (OverlappedWeapon)
	{
		OverlappedWeapon->ShowPickupWidget(true);
	}
	/*if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}*/
	else if (LastWeapon) {
		LastWeapon->ShowPickupWidget(false);
	}
}


//Called on Server Only from WeaponBase.cpp
void AStrykerCharacter::SetOverlappingWeapon(AWeaponBase* Weapon)
{
	if (IsLocallyControlled())
	{
		if (OverlappedWeapon)
		{
			OverlappedWeapon->ShowPickupWidget(false);
		}
	}
	OverlappedWeapon = Weapon;
	if (IsLocallyControlled())//Handles the case when playing as server owned character
	{
		if (OverlappedWeapon)
		{
			OverlappedWeapon->ShowPickupWidget(true);
		}
	}
}
void AStrykerCharacter::SetCrosshair()
{
	if (!LocalPC)return;
	if (!LocalPC->GetGameHUD() && !WeaponComponent->EquippedWeapon->CrosshairImage)return;
	if (!Crosshair)return;
	FSlateBrush Brush;
	Brush.DrawAs = Crosshair->CrosshairLineTop->Brush.DrawAs;
	Brush.Tiling = Crosshair->CrosshairLineTop->Brush.Tiling;
	Brush.Mirroring= Crosshair->CrosshairLineTop->Brush.Mirroring;
	Brush.ImageSize= Crosshair->CrosshairLineTop->Brush.ImageSize;
	Brush.Margin = Crosshair->CrosshairLineTop->Brush.Margin;
	Brush.TintColor= Crosshair->CrosshairLineTop->Brush.TintColor;
	Brush.SetResourceObject(WeaponComponent->EquippedWeapon->CrosshairImage);
	Crosshair->CrosshairLineTop->SetBrush(Brush);
	Crosshair->CrosshairLineBottom->SetBrush(Brush);
	Crosshair->CrosshairLineLeft->SetBrush(Brush);
	Crosshair->CrosshairLineRight->SetBrush(Brush);

	if (LocalPC->GetGameHUD()->ObstacleCrosshair)
	{
		LocalPC->GetGameHUD()->ObstacleCrosshair->CrosshairLineTop->SetBrush(Brush);
		LocalPC->GetGameHUD()->ObstacleCrosshair->CrosshairLineBottom->SetBrush(Brush);
		LocalPC->GetGameHUD()->ObstacleCrosshair->CrosshairLineLeft->SetBrush(Brush);
		LocalPC->GetGameHUD()->ObstacleCrosshair->CrosshairLineRight->SetBrush(Brush);
	}
}
//CALLED ON SERVER 
void AStrykerCharacter::ServerEquip_Implementation()
{
	if (WeaponComponent)
	{
		if (OverlappedWeapon)
		{
			WeaponComponent->EquipWeapon(OverlappedWeapon);
		}
		else if (WeaponComponent->GetShouldSwapWeapons() && IsLocallyControlled() && HasAuthority())
		{
			WeaponComponent->SwapWeapons();
		}
	}


}
void AStrykerCharacter::ServerSwap_Implementation()
{
	if (WeaponComponent)
	{
		WeaponComponent->SwapWeapons();
	}
}


#pragma endregion Interaction



#pragma region Getters
bool AStrykerCharacter::IsWeaponEquiped()
{

	return(	WeaponComponent && WeaponComponent->EquippedWeapon );
}

bool AStrykerCharacter::IsAiming()
{
	return(WeaponComponent && WeaponComponent->bIsAiming);
}

AWeaponBase* AStrykerCharacter::GetEquippedWeapon()
{
	if (WeaponComponent == nullptr) return nullptr;
	return WeaponComponent->EquippedWeapon;
}

FVector AStrykerCharacter::GetHitTarget() const
{
	if (WeaponComponent == nullptr) return FVector();

	return CrosshairLocation;
}





 FRotator AStrykerCharacter::GetShotStartLocAndRot()
{
	const USkeletalMeshSocket* MuzzleSocket = WeaponComponent->EquippedWeapon->GetWeaponMesh()->GetSocketByName("Muzzle");
	FVector ShotLocation = MuzzleSocket->GetSocketTransform(WeaponComponent->EquippedWeapon->GetWeaponMesh()).GetLocation();
	FRotator ShotRotation = UKismetMathLibrary::FindLookAtRotation(ShotLocation, CameraTraceEndLocation);

	return  ShotRotation;
}
 ECombatState AStrykerCharacter::GetCombatState()
 {
	 if (WeaponComponent == nullptr) return ECombatState();
	 return WeaponComponent->CombatState;
	
 }
 bool AStrykerCharacter::GetIsLocallyReloading()
 {
	 if (WeaponComponent == nullptr) return false;
	 return WeaponComponent->GetLocallyReloading();
 }
#pragma endregion Getters


#pragma region BindedInputMethods

void AStrykerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AStrykerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AStrykerCharacter::EventInteract()
{

		if (WeaponComponent->CombatState == ECombatState::ECS_Unoccupied )
			ServerEquip();	

		bool bSwap = WeaponComponent->GetShouldSwapWeapons() &&
			!HasAuthority() &&
			WeaponComponent->CombatState == ECombatState::ECS_Unoccupied &&
			OverlappedWeapon == nullptr;

		if (bSwap)
		{
			ServerSwap();
			//WeaponComponent->bCanFire = true;
			PlayEquipWeaponMontage();
			WeaponComponent->CombatState = ECombatState::ECS_EquipWeapon;
			bFinishedSwapping = false;
		}
}

void AStrykerCharacter::EventReload()
{
	if (WeaponComponent)
		WeaponComponent->Reload();
}

void AStrykerCharacter::EventTossGrenade()
{
	if (WeaponComponent)
		WeaponComponent->TossGrenade();
}

void AStrykerCharacter::EventCrouch()
{
	if(!bIsCrouched)
		Crouch();
	else
	{
		UnCrouch();
	}
}

void AStrykerCharacter::EventAimStart()
{
	if (WeaponComponent)
	{
		WeaponComponent->SetAiming(true);
	}
}

void AStrykerCharacter::EventAimEnd()
{
	if (WeaponComponent)
	{
		 WeaponComponent->SetAiming(false);
	}
}

void AStrykerCharacter::EventFireStart()
{
	if (WeaponComponent )
	{
		WeaponComponent->FireButtonPressed(true);

	}
}

void AStrykerCharacter::EventFireStop()
{
	if (WeaponComponent)
	{
		WeaponComponent->FireButtonPressed(false);
	}
}

#pragma endregion BindedInputMethods











