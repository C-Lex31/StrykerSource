// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawner.h"
#include "PickupBase.h"
// Sets default values
APickupSpawner::APickupSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void APickupSpawner::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnPickupTimer((AActor*)nullptr);
}

void APickupSpawner::SpawnPickup()
{
	int32 NumOfPickupClasses = PickupClasses.Num();
	if (NumOfPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumOfPickupClasses - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickupBase>(PickupClasses[Selection],GetActorTransform());

		if (HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawner::StartSpawnPickupTimer);
		}
	}
}

void APickupSpawner::SpawnPickupTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

void APickupSpawner::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime =bUseRandomSpawnTimes? FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax):SpawnPickupTime;
	GetWorldTimerManager().SetTimer(
		SpawnPickupTimer,
		this,
		&APickupSpawner::SpawnPickupTimerFinished,
		SpawnTime
	);
}

// Called every frame
void APickupSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

