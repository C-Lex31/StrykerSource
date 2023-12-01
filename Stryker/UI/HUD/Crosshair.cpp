// Fill out your copyright notice in the Description page of Project Settings.


#include "Crosshair.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"

void UCrosshair::StoreInitialCrosshairSegmentPos()
{
	//CrosshairLineLeft-
	InitialPosLineLeft = UWidgetLayoutLibrary::SlotAsCanvasSlot(CrosshairLineLeft)->GetPosition().X;
	InitialPosLineRight= UWidgetLayoutLibrary::SlotAsCanvasSlot(CrosshairLineRight)->GetPosition().X;
	InitialPosLineTop= UWidgetLayoutLibrary::SlotAsCanvasSlot(CrosshairLineTop)->GetPosition().Y;
	InitialPosLineBottom = UWidgetLayoutLibrary::SlotAsCanvasSlot(CrosshairLineBottom)->GetPosition().Y;
	
}

void UCrosshair::UpdateDynamicCrosshairVisiblity(ESlateVisibility SlateVisiblity)
{
	CrosshairLineLeft->SetVisibility(SlateVisiblity);
	CrosshairLineRight->SetVisibility(SlateVisiblity);
	CrosshairLineTop->SetVisibility(SlateVisiblity);
	CrosshairLineBottom->SetVisibility(SlateVisiblity);
}

void UCrosshair::UpdateCrosshairPosition(float SpreadScale)
{
	float CrosshairScale = SpreadScale;

	UCanvasPanelSlot* LineLeft = UWidgetLayoutLibrary::SlotAsCanvasSlot(CrosshairLineLeft);
	UCanvasPanelSlot* LineRight = UWidgetLayoutLibrary::SlotAsCanvasSlot(CrosshairLineRight);
	UCanvasPanelSlot* LineTop = UWidgetLayoutLibrary::SlotAsCanvasSlot(CrosshairLineTop);
	UCanvasPanelSlot* LineBottom = UWidgetLayoutLibrary::SlotAsCanvasSlot(CrosshairLineBottom);

	FVector2D LineLeftPos{ InitialPosLineLeft - (150.f * CrosshairScale) , LineLeft->GetPosition().Y };
	FVector2D LineRightPos{ InitialPosLineRight + (150.f * CrosshairScale) , LineRight->GetPosition().Y };
	FVector2D LineTopPos{ LineTop->GetPosition().X ,  InitialPosLineTop - (150.f * CrosshairScale) };
	FVector2D LineBottomPos{ LineBottom->GetPosition().X ,  InitialPosLineBottom + (150.f * CrosshairScale) };

	LineLeft->SetPosition(LineLeftPos);
	LineRight->SetPosition(LineRightPos);
	LineTop->SetPosition(LineTopPos);
	LineBottom->SetPosition(LineBottomPos);
	
}
