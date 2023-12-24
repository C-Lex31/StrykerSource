#pragma once

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252
#define TRACE_LENGTH 80000.f

UENUM(BlueprintType)
enum class EWeaponType :uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
	EWT_SMG UMETA(DisplayName = "SMG"),
	EWT_Sniper UMETA(DisplayName = "SniperRifle"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun")
};
