// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DispenserControlActor.generated.h"

class ADispenserActor;

UCLASS()
class AI_PROJECT_API ADispenserControlActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADispenserControlActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<ADispenserActor*> ControlledDispenser;

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetControlledDispenserLocation();

	float DispenserIterOffsetX = 20;
	float DispenserOffsetX = 0;
};
