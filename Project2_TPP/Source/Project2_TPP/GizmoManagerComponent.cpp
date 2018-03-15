// Fill out your copyright notice in the Description page of Project Settings.

#include "GizmoManagerComponent.h"
#include "ImpulseGadget.h"
#include "Engine/World.h"


// Sets default values for this component's properties
UGizmoManagerComponent::UGizmoManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UGizmoManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UGizmoManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGizmoManagerComponent::SpawnImpulseGadget(FVector location, FRotator rotation, FActorSpawnParameters spawnParameters)
{
	GetWorld()->SpawnActor<AImpulseGadget>(location, rotation, spawnParameters);
}
