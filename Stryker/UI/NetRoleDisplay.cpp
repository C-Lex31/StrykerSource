// Fill out your copyright notice in the Description page of Project Settings.


#include "NetRoleDisplay.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
void UNetRoleDisplay::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UNetRoleDisplay::ShowPlayerNetRole(APawn* InPawn)
{
	FString Name;
	if (!PlayerState)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2,
				FColor::Black,
				FString(TEXT("INVALID PLAYER STATE "))
			);
		}
		return;
	}
	Name = PlayerState->GetPlayerName();
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
		
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	default:
		break;
	}
	FString LocalRoleString = FString::Printf(TEXT("Player :%s Local Role :%s"),*Name, *Role);
	//FString LocalRoleString = FString::Printf(TEXT("%s"), *Name);
	SetDisplayText(LocalRoleString);
}

void UNetRoleDisplay::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}

