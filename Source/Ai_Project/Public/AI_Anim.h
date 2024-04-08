// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AI_Anim.generated.h"

//DECLARE_LOG_CATEGORY_EXTERN(LogMyAnimInstance, Log, All);

UCLASS()
class AI_PROJECT_API UAI_Anim : public UAnimInstance
{
	GENERATED_BODY()

public:
	
	//virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	//FVector CalculateIKPosition(const class AAI_Pawn* Pawn, int32 LandmarkId);
	//void SetIKTarget(const FName& BoneName, const FVector& IKPosition);
};
