// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_Reload.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API UANS_Reload : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
