// Fill out your copyright notice in the Description page of Project Settings.


#include "LSJ/BeverageFluxInterface.h"


// Add default functionality here for any IBeverageFluxInterface functions that are not pure virtual.
void IBeverageFluxInterface::SetBeverage(EBeverage Beverage)
{
	if(Beverage - EBeverage::BeerMax) //Beers
	{
		Viscosity = Beverage / (EBeverage::BeerMax - 1) * 0.2 + 0.8;
		Contrast = Beverage / (EBeverage::BeerMax - 1);
		Form = Beverage / (EBeverage::BeerMax - 1) * 0.2 + 0.8;
	}
	else //Liquors
	{
		Viscosity = Beverage / (EBeverage::BeerMax - 1) * 0.2 + 0.8;
		Contrast = Beverage / (EBeverage::BeerMax - 1);
		Form = Beverage / (EBeverage::BeerMax - 1) * 0.2 + 0.8;
	}
	
}
