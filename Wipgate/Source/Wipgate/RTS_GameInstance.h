// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Helpers/UpgradeHelpers.h"
#include "Helpers/EntityHelpers.h"
#include "RTS_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class WIPGATE_API URTS_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	/* PROPERTIES */
	UPROPERTY(BlueprintReadWrite)
	int CurrentRound = 0;
	UPROPERTY(BlueprintReadWrite)
	TArray<FUpgrade> ActiveUpgrades;
	UPROPERTY(BlueprintReadWrite)
	int CurrentCurrency = 0;
	UPROPERTY(BlueprintReadWrite)
	int CurrentLuma = 0;
	UPROPERTY(BlueprintReadWrite)
		TArray<FEntitySave> SavedEntities;



	/* FUNCTION */
	UFUNCTION(BlueprintCallable)
		void AddCurrency(int amount);
	UFUNCTION(BlueprintCallable)
		void SpendCurrency(int amount);
	UFUNCTION(BlueprintCallable)
		void AddLuma(int amount);
	UFUNCTION(BlueprintCallable)
		void SpendLuma(int amount);
};
