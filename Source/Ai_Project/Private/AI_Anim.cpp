// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Anim.h"
#include "AI_Pawn.h"

//DEFINE_LOG_CATEGORY(LogMyAnimInstance);

/*
void UAI_Anim::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	/*AAI_Pawn* Pawn = Cast<AAI_Pawn>(TryGetPawnOwner());
	if (Pawn)
	{
		//int32 LandmarkId = 

		FName BoneName = Pawn->GetBoneNameFromLandmarkId(LandmarkId);
		FVector IKTargetPosition = CalculateIKPosition(Pawn, LandmarkId);

		SetIKTarget(BoneName, IKTargetPosition);

		UE_LOG(LogMyAnimInstance, Log, TEXT("Updated IK Target Position for Bone: %s"), *BoneName.ToString());
	}#1#
}
*/

/*FVector UMyAnimInstance::CalculateIKPosition(const AAI_Pawn* Pawn, int32 LandmarkId)
{
	if (!Pawn)
		return FVector::ZeroVector;

	return Pawn->GetPositionForLandmarkId(LandmarkId);
}

void UMyAnimInstance::SetIKTarget(const FName& BoneName, const FVector& IKPosition)
{
	USkeletalMeshComponent* SkelMeshComp = GetSkelMeshComponent();

	if (SkelMeshComp && SkelMeshComp->DoesSocketExist(BoneName))
	{
		SkelMeshComp->SetWorldLocation(IKPosition, false, (FHitResult*)nullptr, ETeleportType::TeleportPhysics);

		UE_LOG(LogMyAnimInstance, Log, TEXT("IK position for bone '%s' set to: %s."), *BoneName.ToString(), *IKPosition.ToString());
	}
}*/