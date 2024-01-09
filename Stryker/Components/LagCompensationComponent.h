// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName,FBoxInfo >HitBoxInfo;
};

USTRUCT(BlueprintType)
struct FSSR_Result
{

	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STRYKER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()


public:	
	// Sets default values for this component's properties
	ULagCompensationComponent();

	friend class AStrykerCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);
	FSSR_Result ServerSideRewind(
		 AStrykerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	FSSR_Result ConfirmHit(const FFramePackage& Package,AStrykerCharacter* HitCharacter,const FVector_NetQuantize& TraceStart,const FVector_NetQuantize& HitLocation);

	void CacheBoxPositions(AStrykerCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AStrykerCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AStrykerCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AStrykerCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
private:
	AStrykerCharacter* PlayerCharacter;

	class AStrykerPlayerController* PC;
	
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxFrameRecordDuration = 4.f;
};
