
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Stryker/Enumerations/TurnInPlace.h"
#include "Stryker/Enumerations/CombatState.h"
#include "Stryker/Interfaces/CrosshairInteractableInterface.h"
#include "Stryker/Interfaces/PlayerControllerInterface.h"
#include "Components/TimelineComponent.h"

#include "StrykerCharacter.generated.h"


UCLASS(config=Game)
class AStrykerCharacter : public ACharacter ,public ICrosshairInteractableInterface ,public IPlayerControllerInterface
{
	GENERATED_BODY()

	#pragma region Components
/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWeaponComponent* WeaponComponent;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* BuffComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class ULagCompensationComponent* LCC;

#pragma endregion Components
	
	#pragma region Inputs
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* TossGrenadeAction;
#pragma endregion Inputs

	
	

  #pragma region Animation
	UPROPERTY(EditAnywhere, Category = " AnimMontages")
	class UAnimMontage* FireWeaponMontage;
    UPROPERTY(EditAnywhere, Category = " AnimMontages")
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category = " AnimMontages")
	UAnimMontage* EliminationMontage;
	UPROPERTY(EditAnywhere, Category = " AnimMontages")
	UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, Category = " AnimMontages")
	UAnimMontage* TossGrenadeMontage;
	UPROPERTY(EditAnywhere, Category = " AnimMontages")
	UAnimMontage* WeaponEquipMontage;
#pragma endregion Animation

	UPROPERTY(ReplicatedUsing=OnRep_OverlappedWeapon)
	class AWeaponBase* OverlappedWeapon;
	
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category="UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget>CrosshairClass;

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 100.f;

	bool bFinishedSwapping = false;

	UPROPERTY(EditAnywhere,  Category = Debug)
	float CrossObstacleTraceLength=200.f;

	FVector CameraTraceEndLocation{0.f};
	FVector CrosshairLocation{0.F};
	class UCrosshair* Crosshair;
	float CrosshairSpreadScale{ 0.f }, CrosshairVelocityFactor{ 0.f }, CrosshairInAirFactor{ 0.f }, CrosshairAimFactor{ 0.f }, CrosshairShootingFactor{ 0.f };
	bool bCrosshairHasObstacle{ false };

    #pragma region TimerHandles
    FTimerHandle TH_CenterCrosshair;
	FTimerHandle TH_EliminationTimer;
#pragma endregion TimerHandles

	float AO_Yaw{ 0.f }, AO_Pitch{0.f} , InterpAO_Yaw{ 0.f };
	
	FRotator StartingAimRotation;
	ETurnInPlace TurningInPlace;

	void TurnInPlace(float DeltaTime);	
	void OccludeCharacter();//Hides Player when too close to camera

	UFUNCTION()
	void OnRep_OverlappedWeapon(AWeaponBase* LastWeapon);

	UFUNCTION(Server , Reliable)
	void ServerEquip();

	UFUNCTION(Server, Reliable)
	void ServerSwap();
	
	#pragma region HealthAndElimination
float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing=OnRep_Health ,VisibleAnywhere,Category="PlayerStats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/**
* Player shield
*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Player Stats")
	float Shield = 100.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	bool bEliminated = false;
	void OnElimTimerFinished();

	UPROPERTY(EditDefaultsOnly)
	float RespawnDelay = 3.f;

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
#pragma endregion HealthAndElimination

	/**
* Grenade
*/

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/**
* Default weapon
*/

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeaponBase> DefaultWeaponClass;
	UPROPERTY(EditAnywhere)
	bool bSpawnWithDefaultWeapon = false;

	class AStrykerPlayerController* PC ;
	class AStrykerPlayerState* PS;
	AStrykerPlayerController* LocalPC;


public:
	UPROPERTY()
	TMap<FName, class UBoxComponent*>HitCollisionBoxes;
	/**
	* Hit boxes used for server-side rewind
	*/

	UPROPERTY(EditAnywhere)
    UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;


public:
	AStrykerCharacter();

	#pragma region InlineGetters
/** Returns CameraBoom subobject **/                                        
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }
	FORCEINLINE ULagCompensationComponent* GetLCC() const { return LCC; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurnInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE bool GetIsEliminated() const { return bEliminated; }
	FORCEINLINE bool GetCrosshairHasObstacle() const { return bCrosshairHasObstacle; }
	FORCEINLINE bool GetFinishedSwapping()const { return bFinishedSwapping; }
	FORCEINLINE void SetFinishedSwapping(bool value ) {  bFinishedSwapping =value; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE float GetHealth() { return Health ; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() { return MaxHealth; }
	FORCEINLINE float GetShield() { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetMaxShield() { return MaxShield; }
#pragma endregion InlineGetters

    #pragma region Getters

	bool IsWeaponEquiped();
	bool IsAiming();
	AWeaponBase* GetEquippedWeapon();
	FVector GetHitTarget() const;
	FRotator GetShotStartLocAndRot();
	ECombatState GetCombatState();
	bool GetIsLocallyReloading();
#pragma endregion Getters

    #pragma region Elimination

    void ServerEliminated();//Called from game mode

	UFUNCTION(Client ,Reliable)
	void Client_Eliminated();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated();

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

#pragma endregion Elimination

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;

	void SetOverlappingWeapon(AWeaponBase* Weapon);
	void SetCrosshair();
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void SpawnDefaultWeapon();

	void PlayEliminationMontage();
	void PlayHitReactMontage();
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayTossGrenadeMontage();
	void PlayEquipWeaponMontage();
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget (bool bShowScope);
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			
	void EventInteract();
	void EventReload();
	void EventTossGrenade();
	void EventCrouch();
	void EventAimStart();
	void EventAimEnd();
	void EventFireStart();
	void EventFireStop();
	void AimOffset(float DeltaTime);
	
	void CrosshairLogicUpdate();
	void UpdateCrosshair(float DeltaTime);
	void CenterCrosshair();
	
	void DropOrDestroyWeapon(AWeaponBase* Weapon);
	void DropOrDestroyWeapons();
	FVector TraceCameraAim();
	
	UFUNCTION(Client ,Reliable)
	void InitializeCrosshair();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds) override;


	
};

