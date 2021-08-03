// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProneSystemCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// AProneSystemCharacter

AProneSystemCharacter::AProneSystemCharacter()
{

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.0f, 60.0f, 80.0f));
	CameraBoom->TargetArmLength = 200.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bInheritPitch = true;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bInheritYaw = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetMesh()->SetupAttachment(RootComponent);
	GetMesh()->SetIsReplicated(true);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -97.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName("CharacterMesh");
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>FULLBODY_SKELETALMESH(TEXT("SkeletalMesh'/Game/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin'"));
	if (FULLBODY_SKELETALMESH.Succeeded()) { GetMesh()->SetSkeletalMesh(FULLBODY_SKELETALMESH.Object); }
	static ConstructorHelpers::FClassFinder<UAnimInstance>FULLBODY_ANIMBP(TEXT("AnimBlueprint'/Game/Mannequin/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C'"));
	if (FULLBODY_ANIMBP.Succeeded()) { GetMesh()->SetAnimInstanceClass(FULLBODY_ANIMBP.Class); }

}

void AProneSystemCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Prone", IE_Released, this, &AProneSystemCharacter::PlayerProne);

	PlayerInputComponent->BindAxis("MoveForward", this, &AProneSystemCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProneSystemCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AProneSystemCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProneSystemCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AProneSystemCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProneSystemCharacter::LookUpAtRate);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &AProneSystemCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AProneSystemCharacter::TouchStopped);

}


void AProneSystemCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AProneSystemCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AProneSystemCharacter::PlayerProne()
{
	if (!IsProne) {
		IsProne = true;
		CameraBoom->SetRelativeLocation(FVector(0.0f, 60.0f, 0.0f));
	}
	else {
		IsProne = false;
		CameraBoom->SetRelativeLocation(FVector(0.0f, 60.0f, 80.0f));
	}
}

void AProneSystemCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProneSystemCharacter::LookUpAtRate(float Rate)
{
	float PitchRate = Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds();
	if (IsProne) {
		FRotator PitchAngle = (GetControlRotation() - GetActorRotation()).GetNormalized();
		float UpperPitch = FMath::ClampAngle(PitchAngle.Pitch, -90.0f, 90.0f);
		//PitchLimite = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 180.0f * ProneYawDir), FVector2D(-20.0f, -25.0f), ProneYaw);
		if ((UpperPitch <= -25.0f) && PitchRate >= 0.0f) {
			PitchRate = 0.0f;
		}
		if ((UpperPitch >= 25.0f) && PitchRate <= 0.0f) {
			PitchRate = 0.0f;
		}
		//UE_LOG(LogTemp, Warning, TEXT("UpperPitch: %f"), UpperPitch);
	}
	AddControllerPitchInput(PitchRate);
	//UE_LOG(LogTemp, Warning, TEXT("PitchRate: %f"), PitchRate);

}

void AProneSystemCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AProneSystemCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}
