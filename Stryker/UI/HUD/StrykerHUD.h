// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "StrykerHUD.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter; 
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairsColor;
};
UCLASS()
class STRYKER_API AStrykerHUD : public AHUD
{
	GENERATED_BODY()
	FHUDPackage HUDPackage;
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread , FLinearColor CrosshairColor);
	//class UCharacterOverlay* PlayerOverlay;
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

protected:
	virtual void BeginPlay() override;
	//void CreateCharacterOverlay();
public:
	virtual void DrawHUD()override;
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
// 	UPROPERTY(EditAnywhere,Category="PlayerStats")
// 		TSubclassOf<UUserWidget>PlayerOverlayClass;
//	FORCEINLINE UCharacterOverlay* GetPlayerOverlay() { return PlayerOverlay; }
};
