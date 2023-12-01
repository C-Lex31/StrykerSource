// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NetRoleDisplay.generated.h"

/**
 * 
 */
UCLASS()
class STRYKER_API UNetRoleDisplay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget ))
	class UTextBlock* DisplayText;
	class APlayerState* PlayerState;

	void SetDisplayText(FString TextToDisplay);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);
protected:
	virtual void NativeDestruct() override;
	
};
