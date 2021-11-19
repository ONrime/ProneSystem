// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProneSystemCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character_AnimInstance.h"
#include "ProneUI_UserWidget.h"
#include "Blueprint/UserWidget.h"

//////////////////////////////////////////////////////////////////////////
// AProneSystemCharacter

AProneSystemCharacter::AProneSystemCharacter()
{
	bReplicates = true;
	//SetReplicates(true);
	//SetReplicateMovement(true);

	GetCapsuleComponent()->InitCapsuleSize(2.f, 96.0f);
	RootComponent = GetCapsuleComponent();

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true; //
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 00.0f, 0.0f);
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
	//GetMesh()->SetIsReplicated(true);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -97.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName("CharacterMesh");
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>FULLBODY_SKELETALMESH(TEXT("SkeletalMesh'/Game/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin'"));
	if (FULLBODY_SKELETALMESH.Succeeded()) { GetMesh()->SetSkeletalMesh(FULLBODY_SKELETALMESH.Object); }
	static ConstructorHelpers::FClassFinder<UAnimInstance>FULLBODY_ANIMBP(TEXT("AnimBlueprint'/Game/Mannequin/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C'"));
	if (FULLBODY_ANIMBP.Succeeded()) { GetMesh()->SetAnimInstanceClass(FULLBODY_ANIMBP.Class); }

	static ConstructorHelpers::FClassFinder<UProneUI_UserWidget>PRONEUI_WIDGET(TEXT("WidgetBlueprint'/Game/PronePlayer/ProneHUD.ProneHUD_C'"));
	if (PRONEUI_WIDGET.Succeeded()) ProneUI_Class = PRONEUI_WIDGET.Class;
}

void AProneSystemCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Prone", IE_Released, this, &AProneSystemCharacter::PlayerProne);
	PlayerInputComponent->BindAction("CameraChange", IE_Released, this, &AProneSystemCharacter::PlayerCameraChange);
	PlayerInputComponent->BindAction("ShowCollision", IE_Released, this, &AProneSystemCharacter::PlayerShowCollision);
	PlayerInputComponent->BindAction("DisableProneIK", IE_Released, this, &AProneSystemCharacter::PlayerDisableProneIK);

	PlayerInputComponent->BindAxis("MoveForward", this, &AProneSystemCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProneSystemCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AProneSystemCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProneSystemCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AProneSystemCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProneSystemCharacter::LookUpAtRate);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &AProneSystemCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AProneSystemCharacter::TouchStopped);

}

bool AProneSystemCharacter::Server_SendIsProne_Validate(bool Send)
{
	return true;
}
void AProneSystemCharacter::Server_SendIsProne_Implementation(bool Send)
{
	IsProne = Send;
}

bool AProneSystemCharacter::NetMulticast_SendCtrlRot_Validate(FRotator Rot)
{
	return true;
}
void AProneSystemCharacter::NetMulticast_SendCtrlRot_Implementation(FRotator Rot)
{
	CtrlRot = Rot;
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
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
		CameraBoom->SetRelativeLocation(FVector(0.0f, 60.0f, 0.0f));
	}
	else {
		IsProne = false;
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		CameraBoom->SetRelativeLocation(FVector(0.0f, 60.0f, 80.0f));
	}
	if(!HasAuthority()) Server_SendIsProne(IsProne);
}

void AProneSystemCharacter::PlayerCameraChange()
{
	if (IsCameraChange)
	{
		IsCameraChange = false; 
		CameraBoom->bInheritPitch = true;
		CameraBoom->bInheritRoll = false;
		CameraBoom->bInheritYaw = true;
		CameraBoom->TargetArmLength = 200.0f;
	}
	else {
		IsCameraChange = true;
		CameraBoom->bInheritPitch = false;
		CameraBoom->bInheritRoll = false;
		CameraBoom->bInheritYaw = false;
		CameraBoom->TargetArmLength = 350.0f;
	}
}

void AProneSystemCharacter::PlayerShowCollision()
{
	if (IsShowCollision)
	{
		IsShowCollision = false;
	}
	else 
	{
		IsShowCollision = true;
	}
}

void AProneSystemCharacter::PlayerDisableProneIK()
{
	if (IsProneIK)
	{
		IsProneIK = false;
		ProneUI_WB->ProneStateText = "Prone IK OFF";
	}
	else 
	{
		IsProneIK = true;
		ProneUI_WB->ProneStateText = "Prone IK ON";
	}
}

void AProneSystemCharacter::TurnAtRate(float Rate) // 어차피 클라이언테에서만 작동
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());

	if (Rate != 0.0f)
	{

	}
	
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

void AProneSystemCharacter::BeginPlay()
{
	Super::BeginPlay();

	AnimIns = Cast<UCharacter_AnimInstance>(GetMesh()->GetAnimInstance());

	ProneUI_WB = CreateWidget<UProneUI_UserWidget>(GetWorld(), ProneUI_Class);
	if (ProneUI_WB) {
		ProneUI_WB->AddToViewport();
	}
}

void AProneSystemCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority()) { // 플레이어 전체 회전 (서버)
		if (IsLocallyControlled()) 
		{

		}
		else 
		{
			
		}
		CtrlRot = GetControlRotation();
		NetMulticast_SendCtrlRot(CtrlRot);
		//UE_LOG(LogTemp, Warning, TEXT("server_Updates: %s  CtrlRot %f"), *GetName(), CtrlRot.Yaw);

	}
	else { // (클라이언트)
		if (IsLocallyControlled()) 
		{
			CtrlRot = GetControlRotation();
		}
		else 
		{

		}
		
		//UE_LOG(LogTemp, Warning, TEXT("cla_Updates: %s  CtrlRot %f"), *GetName(), CtrlRot.Yaw);
	}
}

void AProneSystemCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

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

void AProneSystemCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProneSystemCharacter, IsProne);

}
