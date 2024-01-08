// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_Reload.h"
#include "Stryker/Character/StrykerCharacter.h"
void UANS_Reload::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (const AStrykerCharacter* BaseCharacter = Cast<AStrykerCharacter>(MeshComp->GetOwner()))
    {
        if (BaseCharacter->GetWeaponComponent())
        {
            BaseCharacter->GetWeaponComponent()->FinishReloading();
        }
    }
}
