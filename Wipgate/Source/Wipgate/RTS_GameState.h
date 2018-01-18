// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "RTS_GameState.generated.h"

class ARTS_Entity;
class URTS_Team;

DECLARE_LOG_CATEGORY_EXTERN(RTS_Gamestate_log, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEntityDelegate, ARTS_Entity*, entity);

UCLASS()
class WIPGATE_API ARTS_GameState : public AGameState
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
		void RemoveEntity(ARTS_Entity* Entity);

	// All units
	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> Entities;

	// Only selected units
	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> SelectedEntities;


	UFUNCTION(BlueprintCallable)
		float GetPercentOfFriendlyUnitsInLevelGoal();
	UFUNCTION(BlueprintCallable)
		bool AreEnemiesInEndZone();

	// Selection groups
	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> SelectionGroup1;

	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> SelectionGroup2;
	
	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> SelectionGroup3;

	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> SelectionGroup4;

	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> SelectionGroup5;

	UPROPERTY()
	TArray<URTS_Team*> Teams;

	// Event Dispatchers
	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FEntityDelegate OnDeathDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FEntityDelegate UnderAttackDelegate;

	//End screen stats
	UPROPERTY(BlueprintReadOnly, Category = "End screen")
		int UnitsKilled = 0;

	UPROPERTY(BlueprintReadOnly, Category = "End screen")
		int SpecialistsKilled = 0;

	UPROPERTY(BlueprintReadOnly, Category = "End screen")
		int UnitsLost = 0;

	UPROPERTY(BlueprintReadOnly, Category = "End screen")
		int SpecialistsLost = 0;

	UPROPERTY(BlueprintReadOnly, Category = "End screen")
		int LumaGained = 0;

	UPROPERTY(BlueprintReadOnly, Category = "End screen")
		int LumaSpent = 0;

	UPROPERTY(BlueprintReadOnly, Category = "End screen")
		int UnitsOverdosed = 0;
};
