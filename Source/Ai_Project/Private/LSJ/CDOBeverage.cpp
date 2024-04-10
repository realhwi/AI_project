// Fill out your copyright notice in the Description page of Project Settings.


#include "LSJ/CDOBeverage.h"

#include "LSJ/BeverageUnit.h"


// Sets default values
ACDOBeverage::ACDOBeverage()
{
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	
}

// Called when the game starts or when spawned
void ACDOBeverage::BeginPlay()
{
	Super::BeginPlay();
	for (int32 i = 0; CDOSize > i; i++)
	{
		//FActorSpawnParameters _CurrentActorParams;
		CDOBeverage.Push(GetWorld()->SpawnActor<ABeverageUnit>());
	}
}

// Called every frame
void ACDOBeverage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACDOBeverage::NextPoolingBeverage()
{
	CurrentPoolingBeverage++;
}
