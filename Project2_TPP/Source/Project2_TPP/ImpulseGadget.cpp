// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpulseGadget.h"
#include "Engine/World.h"

// Sets default values
AImpulseGadget::AImpulseGadget()
{
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Component"));
	BoxCollider->bGenerateOverlapEvents = true;
	BoxCollider->InitBoxExtent(FVector(50, 50, 50));
	RootComponent = BoxCollider;

	OurParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MovementParticles"));
	OurParticleSystem->SetupAttachment(RootComponent);
	OurParticleSystem->bAutoActivate = false;
	OurParticleSystem->SetRelativeLocation(FVector(-20.0f, 0.0f, 20.0f));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("/Game/StarterContent/Particles/P_Sparks.P_Sparks"));
	if (ParticleAsset.Succeeded())
	{
		OurParticleSystem->SetTemplate(ParticleAsset.Object);
	}

	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &AImpulseGadget::OnOverlapBegin);
	BoxCollider->OnComponentEndOverlap.AddDynamic(this, &AImpulseGadget::OnOverlapEnd);

	impulseAmount = FVector(0.f, 0.f, 100000.0f);
	lifetimeTimerLength = 1.0f;
	timerSet = false;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AImpulseGadget::BeginPlay()
{
	Super::BeginPlay();
	
	for (TActorIterator<AProject2_TPPCharacter> It(GetWorld()); It; ++It)
	{
		playerChar = *It;
	}
}

void AImpulseGadget::lifetimeTimerExpired()
{
	GetWorldTimerManager().ClearTimer(lifetimeTimerHandle);
	Destroy();
}

void AImpulseGadget::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OurParticleSystem && OurParticleSystem->Template)
	{
		OurParticleSystem->ToggleActive();
	}

	UCharacterMovementComponent* movementComp = OtherActor->FindComponentByClass<UCharacterMovementComponent>();
	if (movementComp) {
		movementComp->AddImpulse(impulseAmount);
		if (OtherActor == playerChar) {
			playerChar->canRoll = false;
			if (!timerSet) {
				GetWorld()->GetTimerManager().SetTimer(lifetimeTimerHandle, this, &AImpulseGadget::lifetimeTimerExpired, lifetimeTimerLength, false);
				timerSet = true;
			}
		}
	}
}

void AImpulseGadget::OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	if (OurParticleSystem->IsActive()) {
		OurParticleSystem->ToggleActive();
	}
}

// Called every frame
void AImpulseGadget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

