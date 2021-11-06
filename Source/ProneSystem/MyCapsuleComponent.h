// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "MyCapsuleComponent.generated.h"

/**
 * 
 */
UCLASS()
class PRONESYSTEM_API UMyCapsuleComponent : public UCapsuleComponent
{
	GENERATED_BODY()
	
public:
	UMyCapsuleComponent();

	virtual void PreNetReceive() override;
	virtual void PostRepNotifies() override;

};
