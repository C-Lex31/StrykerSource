// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Crosshair.generated.h"

/**
 * 
 */


UCLASS()
class STRYKER_API UCrosshair : public UUserWidget
{
	GENERATED_BODY()

		float InitialPosLineLeft=0.f;
		float InitialPosLineRight = 0.f;
		float InitialPosLineTop = 0.f;
		float InitialPosLineBottom = 0.f;
public:

	UPROPERTY(meta=(BindWidget))
	class UImage* CrosshairCenter;
	UPROPERTY(meta=(BindWidget))
    UImage* CrosshairLineLeft;
	UPROPERTY(meta=(BindWidget))
	UImage* CrosshairLineRight;
	UPROPERTY(meta=(BindWidget))
	UImage* CrosshairLineTop;
	UPROPERTY(meta=(BindWidget))
	UImage* CrosshairLineBottom;
//	UPROPERTY(meta=(BindWidget))
	float CrosshairSpread;
	//UPROPERTY(meta=(BindWidget))
	FLinearColor CrosshairColor;
	//CrosshairLineLeft->


	void StoreInitialCrosshairSegmentPos();
	void UpdateDynamicCrosshairVisiblity(ESlateVisibility SlateVisiblity);
	void UpdateCrosshairPosition(float SpreadScale );
};
