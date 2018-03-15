// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "Project2_TPPCharacter.h"
#include "TimerManager.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "ImpulseGadget.generated.h"

UCLASS()
class PROJECT2_TPP_API AImpulseGadget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AImpulseGadget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gadget")
	class UBoxComponent *BoxCollider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gadget")
	UParticleSystemComponent *OurParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gadget")
	FVector impulseAmount;

	AProject2_TPPCharacter* playerChar;

	FTimerHandle lifetimeTimerHandle;

	float lifetimeTimerLength; 

	bool timerSet;

	UFUNCTION()
	void lifetimeTimerExpired();

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
