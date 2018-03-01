// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Project2_TPPGameMode.h"
#include "Project2_TPPCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProject2_TPPGameMode::AProject2_TPPGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
