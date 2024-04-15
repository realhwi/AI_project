// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Anim.h"
#include "AI_Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "BoneControllers/AnimNode_ModifyBone.h"


void UAI_Anim::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	// AI_Pawn 인스턴스가 유효한지 확인하고, 본 위치를 업데이트합니다.
	if (!MyAIPawn)
	{
		APawn* PawnOwner = TryGetPawnOwner();
		if (PawnOwner != nullptr)
		{
			MyAIPawn = Cast<AAI_Pawn>(PawnOwner);
		}
	}
    
	if (MyAIPawn)
	{
		UpdateBonePositions();
	}
}

void UAI_Anim::UpdateBonePositions()
{
	/*if (!MyAIPawn) return;

	// 업데이트된 본 위치 정보를 가져옵니다.
	const TMap<int32, FVector>& UpdatedBonePositions = MyAIPawn->GetUpdatedBonePositions();

	for (const TPair<int32, FVector>& BonePosition : UpdatedBonePositions)
	{
		// 스켈레탈 메시에서 본 이름을 가져옵니다.
		FName BoneName = MyAIPawn->GetBoneNameFromLandmarkId(BonePosition.Key, TEXT("Right")); // 또는 "Left"
        
		// 본의 현재 변환(Transform)을 가져옵니다.
		FTransform BoneTransform = SkeletalMeshComponent->GetBoneTransform(SkeletalMeshComponent->GetBoneIndex(BoneName));
        
		// 본의 위치를 업데이트합니다.
		BoneTransform.SetLocation(BonePosition.Value);

		// 컴포넌트의 본 변환을 업데이트합니다.
		SkeletalMeshComponent->SetBoneTransformByIndex(SkeletalMeshComponent->GetBoneIndex(BoneName), BoneTransform, EBoneSpaces::WorldSpace);
	}*/
}


