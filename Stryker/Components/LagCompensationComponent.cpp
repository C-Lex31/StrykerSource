// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Stryker/Character/StrykerCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
// Sets default values for this component's properties
ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// ...
	
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AStrykerCharacter>(GetOwner()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for (auto& BoxPair : PlayerCharacter->HitCollisionBoxes)
		{
			FBoxInfo BI;
			BI.Location = BoxPair.Value->GetComponentLocation();
			BI.Rotation = BoxPair.Value->GetComponentRotation();
			BI.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			Package.HitBoxInfo.Add(BoxPair.Key, BI);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
		false,
			4.f
		);
	}
}

void ULagCompensationComponent::ServerSideRewind(AStrykerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLCC() == nullptr ||
		HitCharacter->GetLCC()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLCC()->FrameHistory.GetTail() == nullptr;

	// Frame package that we check to verify a hit
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;

	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLCC()->FrameHistory; // Reference the Frame history of the HitCharacter
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;

	if (OldestHistoryTime > HitTime)
	{
		// too laggy to perform SSR
		return;
	}
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	while (Older->GetValue().Time > HitTime) // is Older still younger than HitTime?
	{
		if (Older->GetNextNode() == nullptr) break;
		// Traverse back until: OlderTime < HitTime < YoungerTime
		Older = Older->GetNextNode(); 
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if (Older->GetValue().Time == HitTime) // highly unlikely, but we found our frame to check
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	if (bShouldInterpolate)
	{
		// Interpolate between Younger and Older
	}

	if (bReturn) return;
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction= FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxName = YoungerPair.Key;

		const FBoxInfo& OlderBox = OlderFrame.HitBoxInfo[BoxName];
		const FBoxInfo& YoungerBox = YoungerFrame.HitBoxInfo[BoxName]; //YoungerPair[BoxName]

		FBoxInfo InterpBoxInfo;

		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxName, InterpBoxInfo);
	}
	return InterpFramePackage;
}


// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryDuration = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;

		while (HistoryDuration > MaxFrameRecordDuration)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryDuration = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time; //Recalculate History duration
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		ShowFramePackage(ThisFrame, FColor::Red);
		
	}
	// ...
}

