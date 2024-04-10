// Fill out your copyright notice in the Description page of Project Settings.


#include "LSJ/DispenserControlActor.h"

#include "LSJ/DispenserActor.h"

// Sets default values
ADispenserControlActor::ADispenserControlActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	

}

// Called when the game starts or when spawned
void ADispenserControlActor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ADispenserControlActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADispenserControlActor::SetControlledDispenserLocation()
{
	const int32 _NumControlledDispenser = ControlledDispenser.Num();
	for (int i=0; i<_NumControlledDispenser;i++)
	{
		ControlledDispenser[i]->SetActorLocation(this->GetActorLocation() + DispenserIterOffsetX * i + DispenserOffsetX);
	}
}

