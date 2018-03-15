// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Project2_TPPCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include <EngineGlobals.h>
#include "Runtime/Engine/Classes/Camera/CameraTypes.h"
#include "Engine/World.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "GameFramework/SpringArmComponent.h"
#include <GameFramework/Actor.h>
#include <Kismet/GameplayStatics.h>

//////////////////////////////////////////////////////////////////////////
// AProject2_TPPCharacter

AProject2_TPPCharacter::AProject2_TPPCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// gizmo stuff
	GizmoManager = CreateDefaultSubobject<UGizmoManagerComponent>(TEXT("GizmoManager"));

	// enable tick
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Variables
	canRoll = true;
	isRolling = false;
	noForwardMovement = false;
	noRightMovement = false;
	World = GetWorld();
	rollMultiplier = 500.f;
	rollLengthFloat = 0.35f;
	rollCooldownLengthFloat = 2.0f;
	raycastStartOffset = FVector(0.f,0.f,50.f);
	raycastEndOffset = FVector(0.f, 0.f, 550.f);
	distance = 4000.0f;
	finishedLoadingShell = true;
	shotTimer = 2.0f;
	maxHealth = 5.0f;
	gizmoCooldownLengthFloat = 2.0f;
	gizmoReady = true;
	currentHealth = maxHealth;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProject2_TPPCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AProject2_TPPCharacter::checkCanJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Roll", IE_Released, this, &AProject2_TPPCharacter::Roll);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AProject2_TPPCharacter::checkCanShoot);
	PlayerInputComponent->BindAction("Gizmo", IE_Pressed, this, &AProject2_TPPCharacter::CheckCanUseGizmo);

	PlayerInputComponent->BindAxis("MoveForward", this, &AProject2_TPPCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProject2_TPPCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProject2_TPPCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProject2_TPPCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AProject2_TPPCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AProject2_TPPCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AProject2_TPPCharacter::OnResetVR);
}

void AProject2_TPPCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AProject2_TPPCharacter::ResetGizmo()
{
	GetWorldTimerManager().ClearTimer(gizmoCooldownTimerHandle);
	gizmoReady = true;
}

void AProject2_TPPCharacter::ShotTimerFinished()
{
	GetWorldTimerManager().ClearTimer(shotTimerHandle);
	finishedLoadingShell = true;
}

void AProject2_TPPCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AProject2_TPPCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AProject2_TPPCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProject2_TPPCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AProject2_TPPCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (isRolling) {
		if (noForwardMovement && noRightMovement) {			
			AddMovementInput(GetControlRotation().Vector(), rollMultiplier);
		} else {
			AddMovementInput(GetActorForwardVector(), rollMultiplier);
		}
	}
}

void AProject2_TPPCharacter::checkCanJump()
{
	if (isRolling) {
		return;
	}
	else {
		Jump();
	}
}
void AProject2_TPPCharacter::checkCanShoot()
{
	bool canFire = !isRolling && !IsJumpProvidingForce() && !GetCharacterMovement()->IsFalling() && finishedLoadingShell;
	if (canFire) {
		// add timer or ammo, etc
		Shoot();
	}
}
void AProject2_TPPCharacter::Shoot()
{
	FHitResult* hitResult = new FHitResult();
	FVector startTrace = (GetActorLocation() + raycastStartOffset);
	FVector forwardVector = GetFollowCamera()->GetForwardVector();
	FVector endTrace = ((forwardVector * distance) + startTrace + raycastEndOffset);
	FCollisionQueryParams* traceParams = new FCollisionQueryParams();

	if (World->LineTraceSingleByChannel(*hitResult, startTrace, endTrace, ECC_Visibility, *traceParams)) {
		DrawDebugLine(World, startTrace, endTrace, FColor::Green, false, 5.0f);
	/*	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("We shot stuff."));*/
	}
	finishedLoadingShell = false;
	World->GetTimerManager().SetTimer(shotTimerHandle, this, &AProject2_TPPCharacter::ShotTimerFinished, shotTimer, false);
}

void AProject2_TPPCharacter::CustomTakeDamage(int incomingDamage)
{
	currentHealth -= incomingDamage;
	if (currentHealth <= 0) {
		Die();
	}

}

void AProject2_TPPCharacter::Die()
{
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

void AProject2_TPPCharacter::ThrowGizmo()
{
	if (GizmoManager) {
		FVector Offset = GetActorForwardVector() * 200.0f;
		FVector globalLocation = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z - 75.0f);
		FVector Location(Offset + globalLocation);
		FRotator Rotation(GetActorRotation());
		FActorSpawnParameters SpawnInfo;
		GizmoManager->SpawnImpulseGadget(Location, Rotation, SpawnInfo);
		
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Gizmo actor NOT assigned."));
	}

}

void AProject2_TPPCharacter::CheckCanUseGizmo()
{	
	if (gizmoReady) {
		gizmoReady = false;
		World->GetTimerManager().SetTimer(gizmoCooldownTimerHandle, this, &AProject2_TPPCharacter::ResetGizmo, gizmoCooldownLengthFloat, false);
		ThrowGizmo();
	}	
}

void AProject2_TPPCharacter::Jump()
{
	Super::Jump();
	canRoll = false;
}

void AProject2_TPPCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	// if roll hasn't been called
	if (!GetWorldTimerManager().IsTimerActive(rollCooldownTimerHandle)) {
		canRoll = true;
	}
	else {
	}
}


void AProject2_TPPCharacter::Roll()
{
	if (canRoll) {
		canRoll = false;
		isRolling = true;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Call Roll!"));
		World->GetTimerManager().SetTimer(rollLengthTimerHandle, this, &AProject2_TPPCharacter::EndRoll, rollLengthFloat, false);
		World->GetTimerManager().SetTimer(rollCooldownTimerHandle, this, &AProject2_TPPCharacter::ResetRoll, rollCooldownLengthFloat, false);
		GetCharacterMovement()->MaxWalkSpeed = 1600.f;
		GetCharacterMovement()->MaxFlySpeed = 1600.f;
		GetCharacterMovement()->MaxAcceleration = 99999.f;
	}
}

void AProject2_TPPCharacter::EndRoll()
{
	isRolling = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;
	GetCharacterMovement()->MaxAcceleration = 2048.f;
	World->GetTimerManager().ClearTimer(rollLengthTimerHandle);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("End Roll!"));
}

void AProject2_TPPCharacter::ResetRoll()
{
	canRoll = true;
	World->GetTimerManager().ClearTimer(rollCooldownTimerHandle);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Reset Roll!"));
}

void AProject2_TPPCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		noForwardMovement = false;
	}
	else {
		noForwardMovement = true;
	}
}

void AProject2_TPPCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);

		noRightMovement = false;
	}
	else {
		noRightMovement = true;
	}
}


// Example DEBUG Code
//FString test = World->GetName();
//FString* poo = &test;		
//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Some variable values: World: %s"), poo));