// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CDOBeverage.h"
#include "UObject/Interface.h"
#include "BeverageFluxInterface.generated.h"


UENUM()
enum EBeverage : UINT8
{
	Lager,
	Ale,
	Dark,
	BeerMax
};

USTRUCT()
struct FBeverage
{
	GENERATED_BODY()
	float Viscosity = 0; // dropSpeed
	float Contrast = 0; // colour
	float Form = 0; // whip
};




// This class does not need to be modified.
UINTERFACE()
class UBeverageFluxInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AI_PROJECT_API IBeverageFluxInterface
{
	GENERATED_BODY()
	
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	

	UFUNCTION()
	virtual void SetEBeverage(FBeverage& SelfBeverage,EBeverage BeverageCharacter);
	
	UFUNCTION()
	virtual void DropBeverage(FBeverage BeverageStruct, ACDOBeverage* ArrayCDOBeverage);
};
