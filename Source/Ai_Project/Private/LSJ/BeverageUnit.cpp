// Fill out your copyright notice in the Description page of Project Settings.


#include "LSJ/BeverageUnit.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
ABeverageUnit::ABeverageUnit()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	ProjectileMoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

}

// Called when the game starts or when spawned
void ABeverageUnit::BeginPlay()
{
	Super::BeginPlay();
	
	this->SetActorHiddenInGame(true);
	ProjectileMoveComp->SetActive(false);
}

// Called every frame
void ABeverageUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

