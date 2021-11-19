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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	float ProneIK_Upper = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	FRotator ProneIK_Upper_Rot = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	FRotator ProneIK_Pelvis_Rot = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	FRotator ProneIK_Right_Knee_Rot = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	FRotator ProneIK_Left_Knee_Rot = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	FRotator ProneIK_Right_Foot_Rot = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	FRotator ProneIK_Left_Foot_Rot = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	FRotator ProneIK_Right_Hand_Rot = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	FRotator ProneIK_Left_Hand_Rot = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	FVector ProneIK_Right_Hand_Loc = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim, Meta = (AllowPrivateAccess = true))
	FVector ProneIK_Left_Hand_Loc = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Test, Meta = (AllowPrivateAccess = true))
	FRotator TurnDir = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Test, Meta = (AllowPrivateAccess = true))
	FRotator TurnDirEnd = FRotator::ZeroRotator;
	bool IsTurn = false;
	bool IsProneBackMove = false;

	void TurnBodyYaw(class AProneSystemCharacter* Player, float& Yaw, float& ProneYaw);

	/* Prone IK
	* 엎드렸을때 지형지물마다 각도를 맞추는 기능입니다.
	* ProneRootIK, ProneMiddleIK, ProneEndIK 총 3단계를 거치며
	* Root에서 나온 Dir(중심축)에 따라 Middle이 변하고
	* Middle에서 나온 Dir(중심축)에 따라 End가 변합니다.
	*/
	// Prone IK
	FVector ProneRootIK(class AProneSystemCharacter* Player, FName BoneName, FRotator& Rot, bool IsShow);
	FVector ProneMiddleIK(class AProneSystemCharacter* Player, FName BoneName, FRotator& Rot, FVector Dir, bool IsShow);
	void ProneEndIK(class AProneSystemCharacter* Player, FName BoneName, FRotator& Rot, FVector Dir, bool IsShow);
	// 중심축과 지형지물에서 위를 가리키는 축 사이의 각을 구하는 함수입니다.
	float ProneDegrees(float ImpactNomalXY, float ImpactNomalZ, float DirXY, float DirZ, float XY);

	FRotBlend GetProneRotBlend(FVector PlayerForwardLoc, FRotator PlayerRot);

};
