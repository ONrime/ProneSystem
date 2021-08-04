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
		// 컨트롤러와 액터 사이의 pitch 값을 구한다.
		UpperPitch = FMath::ClampAngle(PitchAngle.Pitch, -90.0f, 90.0f);
		// 허리를 180도로 꺽는게 아니기 때문에 90도로 제한을 둔다.

		PlayerSpeed = Player->GetVelocity().Size(); // 플레이어 속도
		IsProne = Player->GetIsProne(); // 엎드리기 상태 구분

		if (IsProne) // 플레이어가 엎드린다면
		{ 
			if (PlayerSpeed > 3.0f)
			{ 
				// 움직일 때 고정된 방향(TurnDirEnd)을 움직이는 방향으로 바꾼다.
				TurnDirEnd = Player->GetActorRotation();

				if (!(UpperProneYaw <= 110.0f && UpperProneYaw >= -110.0f))
				{ 
					// 만약 등이 바닥을 향하게 누워있다면 향해야 되는 방향을 반대로 하기
					TurnDirEnd = FRotator(TurnDirEnd.Pitch, TurnDirEnd.Yaw + 180.0f, TurnDirEnd.Roll);
					IsProneBack = true; // 누워있는 상태
				}
				else {
					IsProneBack = false;  // 엎드려있는 상태
				}
			}
			else
			{ 
				// 움직이지 않을 때 TurnDirEnd을 ActorRotation으로 고정 시킨다.
				if (!IsTurn) // 시작되는 순간에만 고정 되게끔 해야 된다.
				{
					IsTurn = true;
					TurnDirEnd = Player->GetActorRotation();
				}
				if (!(UpperProneYaw <= 110.0f && UpperProneYaw >= -110.0f))
				{
					IsProneBack = true; // 누워있는 상태
				}
				else {
					IsProneBack = false; // 엎드려있는 상태
				}
			}

		}
		else {
			if (PlayerSpeed > 3.0f)
			{ // 움직일 때 하체가 따라가게한다.
				TurnDirEnd = Player->GetActorRotation();
				IsTurn = false;
			}
			else
			{ // 움직이지 않을 때 하체가 따로 움직이게 한다.
				if (!IsTurn)
				{
					IsTurn = true;
					TurnDirEnd = Player->GetActorRotation();
				}
				else if (UpperYaw >= 70.0f || UpperYaw <= -70.0f)
				{
					TurnDirEnd = Player->GetActorRotation();
				}
			}
		}
		TurnDir = FMath::RInterpTo(TurnDir, TurnDirEnd, GetWorld()->GetDeltaSeconds(), 5.0f);
		FRotator YawToAngle = (Player->GetActorRotation() - TurnDir).GetNormalized();
		UpperProneYaw = YawToAngle.Yaw; // 엎드리기 상태일 때 는 전체를 회전 함으로 180도 값을 받아야 한다.
		UpperYaw = FMath::ClampAngle(YawToAngle.Yaw, -90.0f, 90.0f);
		//UE_LOG(LogTemp, Warning, TEXT("UpperYaw: %f"), UpperYaw);
		//UE_LOG(LogTemp, Warning, TEXT("UpperProneYaw: %f"), UpperProneYaw);

		// 엎드리기 상태 일 때 위 아래를 바라보면 그에 따라 상체가 움직일 수 있게 하는 값이다.
		ProneRot = GetProneRotBlend(TurnDirEnd.Vector(), Player->GetActorRotation());
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
