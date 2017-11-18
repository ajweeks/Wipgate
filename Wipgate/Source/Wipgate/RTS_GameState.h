// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "RTS_UnitCharacter.h"
#include "RTS_GameState.generated.h"

UCLASS()
class WIPGATE_API ARTS_GameState : public AGameState
{
	GENERATED_BODY()
	
public:
	// Contains all units
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Units")
	TArray<ARTS_UnitCharacter*> Units;

	// Contains only selected units
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Units")
	TArray<ARTS_UnitCharacter*> SelectedUnits;

};
