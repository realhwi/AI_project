// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AI_Pawn.h" 
#include "AI_Anim.generated.h"

UCLASS()
class AI_PROJECT_API UAI_Anim : public UAnimInstance
{
	GENERATED_BODY()

public:
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	AAI_Pawn* MyAIPawn;
	
	UFUNCTION(BlueprintCallable, Category="Hand Tracking")
	void UpdateBonePositions();
	
};
