// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_PlayerController.h"
//#include "Runtime/Engine/Classes/Components/InputComponent.h"

ARTS_PlayerController::ARTS_PlayerController()
{
	//InputComponent->BindAction("Main Click", IE_Released, this, &ARTS_PlayerController::ActionMainClick);

}

void ARTS_PlayerController::Tick(float DeltaSeconds)
{
}

void ARTS_PlayerController::ActionMainClick()
{
	//UE_LOG(LogTemp, Log, TEXT("Action Main Click!"));
}
