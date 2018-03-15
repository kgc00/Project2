// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "GizmoManagerComponent.h"
#include "Project2_TPPCharacter.generated.h"

UCLASS(config=Game)
class AProject2_TPPCharacter : public ACharacter
{
	GENERATED_BODY()
		
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AProject2_TPPCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	virtual void Tick(float DeltaTime);

	virtual void Jump();

	virtual void Landed(const FHitResult& Hit);

	// roll stuff
	void Roll();
	void EndRoll();
	void ResetRoll();

	
	bool noForwardMovement;
	bool noRightMovement;
	bool isRolling;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Roll)
	float rollMultiplier;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Roll)
	float rollLengthFloat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Roll)
	float rollCooldownLengthFloat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	float distance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	FVector raycastStartOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	FVector raycastEndOffset;
	FTimerHandle rollCooldownTimerHandle;
	FTimerHandle rollLengthTimerHandle;
	UGizmoManagerComponent* GizmoManager;
	class AImpulseGadget* impulseGadget;
	bool gizmoReady;
	UFUNCTION()
	void ResetGizmo();
	FTimerHandle gizmoCooldownTimerHandle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = gizmo)
	float gizmoCooldownLengthFloat;
	UWorld* World;
	FMinimalViewInfo* view;
	bool finishedLoadingShell;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	float shotTimer;
	UFUNCTION()
	void ShotTimerFinished();
	FTimerHandle shotTimerHandle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	int maxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shoot)
	int currentHealth;

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	void checkCanJump();

	void checkCanShoot();

	void Shoot();

	void Die();

	void ThrowGizmo();
	
	void CheckCanUseGizmo();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	void CustomTakeDamage(int incomingDamage);
	bool canRoll;
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
