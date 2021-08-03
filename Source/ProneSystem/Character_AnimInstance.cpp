// Fill out your copyright notice in the Description page of Project Settings.


#include "Character_AnimInstance.h"
#include "ProneSystemCharacter.h"

UCharacter_AnimInstance::UCharacter_AnimInstance()
{

}

void UCharacter_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (::IsValid(Pawn)) {
		AProneSystemCharacter* Player = Cast<AProneSystemCharacter>(Pawn);

		FRotator PitchAngle = (Player->GetControlRotation() - Player->GetActorRotation()).GetNormalized();
		UpperPitch = FMath::ClampAngle(PitchAngle.Pitch, -90.0f, 90.0f);

		IsProne = Player->GetIsProne();

		if (IsProne) {
			if (Player->GetVelocity().Size() > 3.0f)
			{ // 움직일 때 하체가 따라가게
				TrunDirEnd = Player->GetActorRotation();
				if (!(UpperProneYaw <= 110.0f && UpperProneYaw >= -110.0f))
				{
					TrunDirEnd = FRotator(TrunDirEnd.Pitch, TrunDirEnd.Yaw + 180.0f, TrunDirEnd.Roll);
					IsProneBack = true;
				}
				else {
					IsProneBack = false;
				}
			}
			else
			{ // 하체 따로
				if (!IsTurn)
				{
					IsTurn = true;
					TrunDirEnd = Player->GetActorRotation();
				}
				if (!(UpperProneYaw <= 110.0f && UpperProneYaw >= -110.0f))
				{
					IsProneBack = true;
				}
				else {
					IsProneBack = false;
				}
			}

		}
		else {
			if (Player->GetVelocity().Size() > 3.0f)
			{ // 움직일 때 하체가 따라가게
				TrunDirEnd = Player->GetActorRotation();
				IsTurn = false;
			}
			else
			{ // 하체 따로
				if (!IsTurn)
				{
					IsTurn = true;
					TrunDirEnd = Player->GetActorRotation();
				}
				else if (UpperYaw >= 70.0f || UpperYaw <= -70.0f)
				{
					TrunDirEnd = Player->GetActorRotation();
				}
			}
		}
		TrunDir = FMath::RInterpTo(TrunDir, TrunDirEnd, GetWorld()->GetDeltaSeconds(), 5.0f);
		FRotator YawToAngle = (Player->GetActorRotation() - TrunDir).GetNormalized();
		UpperYaw = FMath::ClampAngle(YawToAngle.Yaw, -90.0f, 90.0f);
		UpperProneYaw = YawToAngle.Yaw;
		//UE_LOG(LogTemp, Warning, TEXT("UpperYaw: %f"), UpperYaw);
		//UE_LOG(LogTemp, Warning, TEXT("UpperProneYaw: %f"), UpperProneYaw);

		ProneRot = GetProneRotBlend(TrunDirEnd.Vector(), Player->GetActorRotation());
	}
	
}

FRotBlend UCharacter_AnimInstance::GetProneRotBlend(FVector PlayerForwardLoc, FRotator PlayerRot)
{
	PlayerForwardLoc.Normalize();
	FMatrix RotMatrix = FRotationMatrix(PlayerRot);
	FVector ForwardVector = RotMatrix.GetScaledAxis(EAxis::X);
	FVector RightVector = RotMatrix.GetScaledAxis(EAxis::Y);
	FVector NormalizedVel = PlayerForwardLoc.GetSafeNormal2D();

	float DirForward = FVector::DotProduct(ForwardVector, NormalizedVel);
	float DirRight = FVector::DotProduct(RightVector, NormalizedVel);

	//UE_LOG(LogTemp, Warning, TEXT("DirForward: %f"), DirForward);
	//UE_LOG(LogTemp, Warning, TEXT("DirRight: %f"), DirRight);
	FRotBlend SetRot;
	SetRot.Front = FMath::Clamp(DirForward, 0.0f, 1.0f);
	SetRot.Back = FMath::Abs(FMath::Clamp(DirForward, -1.0f, 0.0f));
	SetRot.Left = FMath::Abs(FMath::Clamp(DirRight, -1.0f, 0.0f));
	SetRot.Right = FMath::Clamp(DirRight, 0.0f, 1.0f);

	return SetRot;
}
