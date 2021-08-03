// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProneSystemGameMode.h"
#include "ProneSystemCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProneSystemGameMode::AProneSystemGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
