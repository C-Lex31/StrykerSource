// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h "
#include "Stryker/Character/StrykerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Stryker/PlayerController/StrykerPlayerController.h"
#include "Stryker/Components/WeaponComponent.h"
#include "Engine/SkeletalMeshSocket.h"
void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps((OutLifetimeProps));
	DOREPLIFETIME(AWeaponBase, WeaponState);
	//DOREPLIFETIME(AWeaponBase, Ammo);
}
void AWeaponBase::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		OwnerCharacter = nullptr;
		OwnerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}
// Sets default values
AWeaponBase::AWeaponBase()
{
	bReplicates = true;
	SetReplicateMovement(true);
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	//WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	EnableCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(WeaponMesh);
}


void AWeaponBase::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponBase::OnSphereEndOverlap);

	if ( PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
}



void AWeaponBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(OtherActor);
	if (StrykerCharacter)
	{
		StrykerCharacter->SetOverlappingWeapon((this));
	}
}

void AWeaponBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	AStrykerCharacter* StrykerCharacter = Cast<AStrykerCharacter>(OtherActor);
	if (StrykerCharacter)
	{
		StrykerCharacter->SetOverlappingWeapon((nullptr));
	}
}

// Called On Server
void AWeaponBase::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}
//Not called on Server
void AWeaponBase::OnRep_WeaponState()
{
	OnWeaponStateSet();
}
void AWeaponBase::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_EquippedPrimary:
		OnEquippedPrimary();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	default:
		break;
	}
}
void AWeaponBase::OnEquippedPrimary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnableCustomDepth(false);
}
void AWeaponBase::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}
void AWeaponBase::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EnableCustomDepth(true);
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();
	}
}
//Server Only
void AWeaponBase::SetHUDAmmo()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AStrykerCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter)
	{
		OwnerController = OwnerController == nullptr ? Cast<AStrykerPlayerController>(OwnerCharacter->Controller) : OwnerController;
		if (OwnerController)
		{
			OwnerController->SetWeaponAmmo(Ammo);
		}
	}
}


void AWeaponBase::SpendRound()
{	
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();

	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else if(OwnerCharacter && OwnerCharacter->IsLocallyControlled()) //Spend round is called on both server and client .If not checked , when server fires a weapon , sequence will be updated for weapon on clients machine.
	{
		++Sequence;
	}
}
void AWeaponBase::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}
void AWeaponBase::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
    OwnerCharacter = OwnerCharacter == nullptr ? Cast<AStrykerCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter && OwnerCharacter->GetWeaponComponent() && GetIsFull())
	{
		OwnerCharacter->GetWeaponComponent()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



void AWeaponBase::ShowPickupWidget(bool bPickupWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bPickupWidget);
	}
}



void AWeaponBase::Fire(const FVector& HitTarget)
{
	if (WeaponFireAnimation)
	{
		WeaponMesh->PlayAnimation(WeaponFireAnimation, false);
	}
		SpendRound();
}

void AWeaponBase::DropWeapon()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController =nullptr ;
}

//CALED ON SERVER
void AWeaponBase::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}
FVector AWeaponBase::TraceEndWithScatter( const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	if (MuzzleSocket == nullptr) return FVector();
	FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
	FVector TraceStart = SocketTransform.GetLocation();

	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;

	if (bDrawDebugScatterTrace)
	{
		DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
		DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
		DrawDebugLine(
			GetWorld(),
			TraceStart,
			FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),// Dividing to prevent overflow of x,y,z values as TRACE_LENGTH is a large value.
			FColor::Cyan,
			true);
	}


	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

