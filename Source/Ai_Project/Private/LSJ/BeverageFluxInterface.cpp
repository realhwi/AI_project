// Fill out your copyright notice in the Description page of Project Settings.


#include "LSJ/BeverageFluxInterface.h"

#include "LSJ/CDOBeverage.h"


// Add default functionality here for any IBeverageFluxInterface functions that are not pure virtual.


void IBeverageFluxInterface::SetEBeverage(FBeverage& SelfBeverage,EBeverage BeverageCharacter)
{
	if(BeverageCharacter - EBeverage::BeerMax) //Beers
	{
		SelfBeverage.Viscosity = BeverageCharacter / (EBeverage::BeerMax - 1) * 0.2 + 0.8;
		SelfBeverage.Contrast = BeverageCharacter / (EBeverage::BeerMax - 1);
		SelfBeverage.Form = BeverageCharacter / (EBeverage::BeerMax - 1) * 0.2 + 0.8;
	}
	else //Liquors
	{
		SelfBeverage.Viscosity = BeverageCharacter / (EBeverage::BeerMax - 1) * 0.2 + 0.8;
		SelfBeverage.Contrast = BeverageCharacter / (EBeverage::BeerMax - 1);
		SelfBeverage.Form = BeverageCharacter / (EBeverage::BeerMax - 1) * 0.2 + 0.8;
	}
}
	

void IBeverageFluxInterface::DropBeverage(FBeverage BeverageStruct, ACDOBeverage* ArrayCDOBeverage)
{
	
}
