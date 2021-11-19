// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProneSystemCharacter.generated.h"

UCLASS(config=Game)
class AProneSystemCharacter : public ACharacter
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AProneSystemCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY()
	class UProneUI_UserWidget* ProneUI_WB = nullptr;

	bool GetIsProne() { return IsProne; }
	FRotator GetCtrlRot() { return CtrlRot; }

	bool IsShowCollision = false;
	bool IsProneIK = true;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);
	
	void PlayerProne();
	void PlayerCameraChange();
	void PlayerShowCollision();
	void PlayerDisableProneIK();

	UPROPERTY(Replicated)
	bool IsProne = false;
	bool IsProneBack = false;
	bool IsTurn = false;
	bool IsCameraChange = false;
	//UPROPERTY(ReplicatedUsing = OnRep_CtrlRotChange)
	FRotator CtrlRot = FRotator::ZeroRotator;

	UPROPERTY()
	class UCharacter_AnimInstance* AnimIns = nullptr;


	TSubclassOf<class UProneUI_UserWidget> ProneUI_Class;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendIsProne(bool Send);
	bool Server_SendIsProne_Validate(bool Send);
	void Server_SendIsProne_Implementation(bool Send);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void NetMulticast_SendCtrlRot(FRotator Rot);
	bool NetMulticast_SendCtrlRot_Validate(FRotator Rot);
	void NetMulticast_SendCtrlRot_Implementation(FRotator Rot);

};

