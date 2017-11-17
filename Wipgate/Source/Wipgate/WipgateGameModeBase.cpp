// Fill out your copyright notice in the Description page of Project Settings.

#include "WipgateGameModeBase.h"

AWipgateGameModeBase::AWipgateGameModeBase()
{

}


void AWipgateGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), Main_UI_HUD);

	if (CurrentWidget != nullptr)
	{
		CurrentWidget->AddToViewport(1000);

		
	}
}



