// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "RTS_GameState.generated.h"

class ARTS_Entity;

DECLARE_LOG_CATEGORY_EXTERN(RTS_Gamestate_log, Log, All);

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


	// Selection groups
	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> SelectionGroup1;

	bool SelectionGroup1IconCreated;

	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> SelectionGroup2;
	
	bool SelectionGroup2IconCreated;

	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> SelectionGroup3;

	bool SelectionGroup3IconCreated;

	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> SelectionGroup4;

	bool SelectionGroup4IconCreated;

	UPROPERTY(BlueprintReadWrite, Category = "Units")
		TArray<ARTS_Entity*> SelectionGroup5;

	bool SelectionGroup5IconCreated;
};
