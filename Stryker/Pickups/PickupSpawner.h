// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawner.generated.h"

UCLASS()
class STRYKER_API APickupSpawner : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "SpawnerSettings")
	bool bUseRandomSpawnTimes = true;

	UPROPERTY(EditAnywhere, Category = "SpawnerSettings")
	float SpawnPickupTimeMin;

	UPROPERTY(EditAnywhere, Category = "SpawnerSettings")
	float SpawnPickupTimeMax;

	UPROPERTY(EditAnywhere, Category = "SpawnerSettings")
	float SpawnPickupTime;

	FTimerHandle SpawnPickupTimer;
public:	

	APickupSpawner();

protected:

	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere ,Category ="SpawnerSettings")
	TArray<TSubclassOf<class APickupBase>> PickupClasses;

	APickupBase* SpawnedPickup =nullptr;
	void SpawnPickup();
	void SpawnPickupTimerFinished();

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
public:	

	virtual void Tick(float DeltaTime) override;

};
