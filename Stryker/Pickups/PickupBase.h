// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

UCLASS()
class STRYKER_API APickupBase : public AActor
{
	GENERATED_BODY()
	
    UPROPERTY(EditAnywhere ,Category ="PickupBaseProperties")
	class USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere, Category = "PickupBaseProperties")
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, Category = "PickupBaseProperties")
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(VisibleAnywhere, Category = "PickupBaseProperties")
	class UNiagaraComponent* PickupEffectComponent;

	UPROPERTY(EditAnywhere, Category = "PickupBaseProperties")
	class UNiagaraSystem* PickupEffect;
public:	
	// Sets default values for this actor's properties
	APickupBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

};
