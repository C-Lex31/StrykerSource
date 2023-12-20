#pragma once

#define CUSTOM_DEPTH_PURPLE 250
#define TRACE_LENGTH 80000.f

UENUM(BlueprintType)
enum class EWeaponType :uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
	EWT_SMG UMETA(DisplayName = "SMG")
};
