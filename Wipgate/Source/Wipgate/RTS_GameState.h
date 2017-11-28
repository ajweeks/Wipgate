// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "RTS_UnitCharacter.h"
#include "RTS_GameState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(RTS_Gamestate_log, Log, All);

UCLASS()
class WIPGATE_API ARTS_GameState : public AGameState
{
	GENERATED_BODY()
	
public:
	//FUNCTIONS
	UFUNCTION(BlueprintCallable)
		void RemoveUnit(ARTS_UnitCharacter* unit);

	// All units
	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_UnitCharacter*> Units;

	// Only selected units
	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_UnitCharacter*> SelectedUnits;


	// Selection groups
	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_UnitCharacter*> SelectionGroup1;

	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_UnitCharacter*> SelectionGroup2;

	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_UnitCharacter*> SelectionGroup3;

	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_UnitCharacter*> SelectionGroup4;

	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_UnitCharacter*> SelectionGroup5;

};
