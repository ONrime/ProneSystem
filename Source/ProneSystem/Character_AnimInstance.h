// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character_AnimInstance.generated.h"

/**
 * 
 */

USTRUCT(Atomic, BlueprintType)
struct FRotBlend
{
	GENERATED_USTRUCT_BODY()

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Front;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Back;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Left;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Right;
};

UCLASS()
class PRONESYSTEM_API UCharacter_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UCharacter_AnimInstance();
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	float UpperYaw = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	float UpperProneYaw = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	float UpperPitch = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	float PlayerSpeed = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	bool IsProne = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	bool IsProneBack = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	FRotBlend ProneRot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Test, Meta = (AllowPrivateAccess = true))
	FRotator TurnDir = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Test, Meta = (AllowPrivateAccess = true))
	FRotator TurnDirEnd = FRotator::ZeroRotator;
	bool IsTurn = false;
	bool IsProneBackMove = false;

	FRotBlend GetProneRotBlend(FVector PlayerForwardLoc, FRotator PlayerRot);

};
