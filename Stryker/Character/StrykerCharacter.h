
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Stryker/Types/TurnInPlace.h"
#include "Stryker/Interfaces/CrosshairInteractableInterface.h"
#include "Stryker/Interfaces/PlayerControllerInterface.h"
#include "Components/TimelineComponent.h"
#include "StrykerCharacter.generated.h"


UCLASS(config=Game)
class AStrykerCharacter : public ACharacter ,public ICrosshairInteractableInterface ,public IPlayerControllerInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category="UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget>CrosshairClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* EliminationMontage;

	UPROPERTY(ReplicatedUsing=OnRep_OverlappedWeapon)
	class AWeaponBase* OverlappedWeapon;
	
	UFUNCTION()
	void OnRep_OverlappedWeapon(AWeaponBase* LastWeapon);
	UFUNCTION(Server , Reliable)
	void ServerEquip();
	//Hides Player when too close to camera
	void OccludeCharacter();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 100.f;
	UPROPERTY(EditAnywhere,  Category = Debug)
	float CrossObstacleTraceLength=200.f;
	FVector CameraTraceEndLocation;
	class UCrosshair* Crosshair;
	FTimerHandle TH_CenterCrosshair;
	FTimerHandle TH_EliminationTimer;

	float AO_Yaw{ 0.f }, AO_Pitch{0.f};
	float CrosshairSpreadScale{ 0.f }, CrosshairVelocityFactor{ 0.f }, CrosshairInAirFactor{ 0.f }, CrosshairAimFactor{ 0.f }, CrosshairShootingFactor{ 0.f };

	bool bCrosshairHasObstacle{ false };
	float InterpAO_Yaw{ 0.f };
	
	FRotator StartingAimRotation;
	ETurnInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	bool bEliminated = false;
	void OnElimTimerFinished();
	UPROPERTY(EditDefaultsOnly)
	float RespawnDelay = 3.f;
	/*
		PlayerHealth
	*/
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing=OnRep_Health ,VisibleAnywhere,Category="PlayerStats")
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health();

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* TL_Dissolve;
	FOnTimelineFloat DissolveTrack;
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;
	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicDissolveMaterial;
	UPROPERTY(EditAnywhere)
	UMaterialInstance* DissolveMaterial;
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveAmount);
	void StartDissolve();


	class AStrykerPlayerController* PC ;
	AStrykerPlayerController* LocalPC;
	virtual void I_ClientHUD()override;
public:
	AStrykerCharacter();
		UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWeaponComponent* WeaponComponent;
	/** Returns CameraBoom subobject **/                                        
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurnInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE bool GetIsEliminated() const { return bEliminated; }
	FORCEINLINE bool GetCrosshairHasObstacle() const { return bCrosshairHasObstacle; }
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	void SetOverlappingWeapon(AWeaponBase* Weapon);
	virtual void PossessedBy(AController* NewController) override;
	//UFUNCTION(NetMulticast, Unreliable)
	void PlayHitReactMontage();
	//UFUNCTION(NetMulticast, Unreliable)
	//void MulticastHit();

	virtual void PostInitializeComponents() override;
	bool IsWeaponEquiped();
	bool IsAiming();
	AWeaponBase* GetEquippedWeapon();
	FVector GetHitTarget() const;
	void ServerEliminated();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated();
	void PlayEliminationMontage();
	 FRotator GetShotStartLocAndRot();
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			
	void EventInteract();
	void EventCrouch();
	void EventAimStart();
	void EventAimEnd();
	void EventFireStart();
	void EventFireStop();
	void AimOffset(float DeltaTime);
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	void UpdateHUDHealth();
	FVector TraceCameraAim();
	UFUNCTION(Client , Reliable)
	void CrosshairLogicUpdate();
	void UpdateCrosshair(float DeltaTime);
	UFUNCTION(Client ,Reliable)
	void InitializeCrosshair();
	void CenterCrosshair();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds) override;


	
};

