// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProneUI_UserWidget.generated.h"

/**
 * 
 */
UCLASS()
class PRONESYSTEM_API UProneUI_UserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prone")
	FString ProneStateText = "Prone IK ON";

};
