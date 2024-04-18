// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Anim.h"
#include "AI_Pawn.h"
#include "Components/SkeletalMeshComponent.h"

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
}


