// Fill out your copyright notice in the Description page of Project Settings.


#include "Character_AnimInstance.h"
#include "ProneSystemCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

UCharacter_AnimInstance::UCharacter_AnimInstance()
{

}

void UCharacter_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (::IsValid(Pawn)) {
		AProneSystemCharacter* Player = Cast<AProneSystemCharacter>(Pawn);

		TurnBodyYaw(Player, UpperYaw, UpperProneYaw);
		// 컨트롤러와 액터 사이의 pitch 값을 구한다.
		FRotator PitchAngle = (Player->GetCtrlRot() - Player->GetActorRotation()).GetNormalized();
		// 허리를 180도로 꺽는게 아니기 때문에 90도로 제한을 둔다.
		UpperPitch = FMath::ClampAngle(PitchAngle.Pitch, -90.0f, 90.0f);
		
		PlayerSpeed = Player->GetVelocity().Size(); // 플레이어 속도
		IsProne = Player->GetIsProne(); // 엎드리기 상태 구분

		if (IsProne && Player->IsProneIK)
		{
			// Root
			FVector RootDir = ProneRootIK(Player, "pelvis", ProneIK_Pelvis_Rot, Player->IsShowCollision);

			// 척추
			FVector SpineDir = ProneMiddleIK(Player, "ProneSpineLoc", ProneIK_Upper_Rot, RootDir, Player->IsShowCollision);
			// 무릎
			FVector Calf_Rgiht_Dir = ProneMiddleIK(Player, "ProneCalfRightLoc", ProneIK_Right_Knee_Rot, RootDir, Player->IsShowCollision);
			FVector Calf_Left_Dir = ProneMiddleIK(Player, "ProneCalfLeftLoc", ProneIK_Left_Knee_Rot, RootDir, Player->IsShowCollision);

			// 발
			ProneEndIK(Player, "ProneFootRightLoc", ProneIK_Right_Foot_Rot, Calf_Rgiht_Dir, Player->IsShowCollision);
			ProneEndIK(Player, "ProneFootLeftLoc", ProneIK_Left_Foot_Rot, Calf_Left_Dir, Player->IsShowCollision);
			// 팔꿈치
			ProneEndIK(Player, "ProneArmRightLoc", ProneIK_Right_Hand_Rot, SpineDir, Player->IsShowCollision);
			ProneEndIK(Player, "ProneArmLeftLoc", ProneIK_Left_Hand_Rot, SpineDir, Player->IsShowCollision);
		}
	}
	
}

void UCharacter_AnimInstance::TurnBodyYaw(AProneSystemCharacter* Player, float& Yaw, float& ProneYaw)
{
	float YawEnd = 0.0f;
	if (Player->GetIsProne()) // 플레이어가 엎드린다면
	{
		if (Player->GetVelocity().Size() > 3.0f)
		{
			// 움직일 때 고정된 방향(TurnDirEnd)을 움직이는 방향으로 바꾼다.
			TurnDirEnd = Player->GetActorRotation();

			if (!(ProneYaw <= 110.0f && ProneYaw >= -110.0f))
			{
				// 만약 등이 바닥을 향하게 누워있다면 향해야 되는 방향을 반대로 하기
				TurnDirEnd = FRotator(TurnDirEnd.Pitch, TurnDirEnd.Yaw + 180.0f, TurnDirEnd.Roll);
				IsProneBack = true; // 누워있는 상태
			}
			else 
			{
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
			if (!(ProneYaw <= 110.0f && ProneYaw >= -110.0f))
			{
				IsProneBack = true; // 누워있는 상태
			}
			else {
				IsProneBack = false; // 엎드려있는 상태
			}
		}

		// 엎드리기 상태 일 때 위 아래를 바라보면 그에 따라 상체가 움직일 수 있게 하는 값이다.
		ProneRot = GetProneRotBlend(TurnDirEnd.Vector(), Player->GetActorRotation());
	}
	else {
		if (Player->GetVelocity().X > 0.0f || Player->GetVelocity().Y > 0.0f) 
		{ // 움직일 때 하체가 따라가게한다.
			TurnDir = Player->GetActorRotation();
			TurnDirEnd = Player->GetActorRotation();
			IsTurn = false;
		}
		else 
		{ // 움직이지 않을 때 하체가 따로 움직이게 한다.
			if (!IsTurn) {
				IsTurn = true;
				TurnDirEnd = Player->GetActorRotation();
			}
			else {
				if (Yaw >= 70.0f || Yaw <= -70.0f) {
					TurnDirEnd = Player->GetActorRotation();
				}
			}
		}
	}
	TurnDir = FMath::RInterpTo(TurnDir, TurnDirEnd, GetWorld()->GetDeltaSeconds(), 5.0f);
	FRotator interpToAngle = (Player->GetActorRotation() - TurnDir).GetNormalized();
	ProneYaw = interpToAngle.Yaw; // 엎드리기 상태일 때 는 전체를 회전 함으로 180도 값을 받아야 한다.
	YawEnd = FMath::ClampAngle(interpToAngle.Yaw, -90.0f, 90.0f);
	Yaw = -YawEnd;

}

FVector UCharacter_AnimInstance::ProneRootIK(AProneSystemCharacter* Player, FName BoneName, FRotator& Rot, bool IsShow)
{
	FVector SocketLoc = Player->GetMesh()->GetSocketLocation(BoneName);
	FVector StartTracer = SocketLoc + FVector(0.0f, 0.0f, 20.0f);
	FVector EndTracer = SocketLoc - FVector(0.0f, 0.0f, 60.0f);
	TArray<AActor*> ActorsToIgnore;
	FHitResult OutHit;

	bool hitis = false;
	if (IsShow)
	{
		hitis = UKismetSystemLibrary::SphereTraceSingle(this, StartTracer, EndTracer, 3.0f,
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
			EDrawDebugTrace::ForOneFrame, OutHit, true);
	}
	else 
	{
		hitis = UKismetSystemLibrary::SphereTraceSingle(this, StartTracer, EndTracer, 3.0f,
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
			EDrawDebugTrace::None, OutHit, true);
	}

	if (hitis)
	{
		FVector ImpactNomal = OutHit.ImpactNormal;
		/*UKismetSystemLibrary::SphereTraceSingle(this, ImpactNomal * 40.0f + SocketLoc, ImpactNomal * -40.0f + SocketLoc, 3.0f,
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
			EDrawDebugTrace::ForOneFrame, OutHit, true);*/
		//UE_LOG(LogTemp, Warning, TEXT("ImpactNomal: %f, %f, %f"), ImpactNomal.X, ImpactNomal.Y, ImpactNomal.Z);
		float ProneRotationRoll = FMath::RadiansToDegrees(FMath::Atan2(ImpactNomal.Y, ImpactNomal.Z));
		float ProneRotationPitch = FMath::RadiansToDegrees(FMath::Atan2(ImpactNomal.X, ImpactNomal.Z) * -1.0f);
		Rot = FMath::RInterpTo(Rot, FRotator(ProneRotationPitch, 0.0f, ProneRotationRoll), GetWorld()->GetDeltaSeconds(), 15.0f);
		return ImpactNomal;
	}

	return FVector::ZeroVector;
}

FVector UCharacter_AnimInstance::ProneMiddleIK(AProneSystemCharacter* Player, FName BoneName, FRotator& Rot, FVector Dir, bool IsShow)
{
	FVector SocketLoc = Player->GetMesh()->GetSocketLocation(BoneName);
	FVector StartTracer = SocketLoc + Dir * 20.0f;
	FVector EndTracer = SocketLoc - Dir * 60.0f;
	TArray<AActor*> ActorsToIgnore;
	FHitResult OutHit;

	bool hitis = false;
	if (IsShow)
	{
		hitis = UKismetSystemLibrary::SphereTraceSingle(this, StartTracer, EndTracer, 3.0f,
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
			EDrawDebugTrace::ForOneFrame, OutHit, true);
	}
	else
	{
		hitis = UKismetSystemLibrary::SphereTraceSingle(this, StartTracer, EndTracer, 3.0f,
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
			EDrawDebugTrace::None, OutHit, true);
	}

	if (hitis)
	{
		FVector ImpactNomal = OutHit.ImpactNormal;
		float ProneRotationRoll = ProneDegrees(ImpactNomal.Y, ImpactNomal.Z, Dir.Y, Dir.Z, 1.0f);
		float ProneRotationPitch = ProneDegrees(ImpactNomal.X, ImpactNomal.Z, Dir.X, Dir.Z, -1.0f);
		Rot = FMath::RInterpTo(Rot, FRotator(ProneRotationPitch, 0.0f, ProneRotationRoll), GetWorld()->GetDeltaSeconds(), 15.0f);
		return ImpactNomal;
	}

	return FVector::ZeroVector;
}


void UCharacter_AnimInstance::ProneEndIK(AProneSystemCharacter* Player, FName BoneName, FRotator& Rot, FVector Dir, bool IsShow)
{
	FVector SocketLoc = Player->GetMesh()->GetSocketLocation(BoneName);
	FVector StartTracer = SocketLoc + Dir * 20.0f;
	FVector EndTracer = SocketLoc - Dir * 60.0f;

	FRotator Test = FRotator(0.0f, 0.0f, 5.0f);

	TArray<AActor*> ActorsToIgnore;
	FHitResult OutHit;

	bool hitis = false;
	if (IsShow)
	{
		hitis = UKismetSystemLibrary::SphereTraceSingle(this, StartTracer, EndTracer, 3.0f,
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
			EDrawDebugTrace::ForOneFrame, OutHit, true);
	}
	else
	{
		hitis = UKismetSystemLibrary::SphereTraceSingle(this, StartTracer, EndTracer, 3.0f,
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
			EDrawDebugTrace::None, OutHit, true);
	}

	if (hitis)
	{
		FVector ImpactNomal = OutHit.ImpactNormal;
		/*UKismetSystemLibrary::SphereTraceSingle(this, ImpactNomal * 60.0f + SocketLoc, ImpactNomal * -60.0f + SocketLoc, 3.0f,
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
			EDrawDebugTrace::ForOneFrame, OutHit, true);*/
		float ProneRotationRoll = ProneDegrees(ImpactNomal.Y, ImpactNomal.Z, Dir.Y, Dir.Z, 1.0f);
		float ProneRotationPitch = ProneDegrees(ImpactNomal.X, ImpactNomal.Z, Dir.X, Dir.Z, -1.0f);
		Rot = FMath::RInterpTo(Rot, FRotator(ProneRotationPitch, 0.0f, ProneRotationRoll), GetWorld()->GetDeltaSeconds(), 15.0f);
	}
}

float UCharacter_AnimInstance::ProneDegrees(float ImpactNomalXY, float ImpactNomalZ, float DirXY, float DirZ, float XY)
{
	float Degrees = 0.0f;
	if (DirZ > ImpactNomalZ)
	{
		Degrees = FMath::RadiansToDegrees(FMath::Atan2(ImpactNomalXY, DirZ) * XY);
	}
	else if (DirZ < ImpactNomalZ)
	{
		Degrees = FMath::RadiansToDegrees(FMath::Atan2(DirXY, ImpactNomalZ) * -1.0f * XY);
	}

	return Degrees;
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
