// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Engine/DataTable.h"
#include "Helpers/EntityHelpers.h"
#include "WipgateGameModeBase.generated.h"

class URTS_Team;
class ARTS_PlayerSpawner;
class ARTS_LevelEnd;
class ARTS_LevelBounds;
class AUpgradeShopBase;

DECLARE_LOG_CATEGORY_EXTERN(WipgateGameModeBase, Log, All);

UCLASS()
class WIPGATE_API AWipgateGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	URTS_Team* GetTeamWithAlignment(ETeamAlignment alignment);

	UFUNCTION(BlueprintCallable)
		void SaveResources();

	UFUNCTION(BlueprintCallable)
		void NextLevel();

	UFUNCTION(BlueprintCallable)
		ARTS_PlayerSpawner* GetPlayerSpawner();

	UFUNCTION(BlueprintCallable)
		ARTS_LevelEnd* GetLevelEnd();

	UFUNCTION(BlueprintCallable)
		ARTS_LevelBounds* GetLevelBounds();

	UFUNCTION(BlueprintCallable)
		const TArray<AUpgradeShopBase*>& GetShops();

private:
	//Make sure the datatable is inheriting from FTeamRow
	UPROPERTY(EditAnywhere)
	UDataTable* m_TeamTable;

	//Make sure the datatable is inheriting from FEnemyUpgrade
	UPROPERTY(EditAnywhere)
	UDataTable* m_EnemyUpgradeTable;

	//Make sure the datatable is inheriting from FEntityRow
	UPROPERTY(EditAnywhere)
		UDataTable* m_FriendlyAddedTroops;

	//Base spawn chance of an entity spawner
	UPROPERTY(EditAnywhere)
		float BaseSpawnChance = 0.5f;

	//Spawn chance increase per round
	UPROPERTY(EditAnywhere)
		float SpawnChanceRoundIncrease = 0.05f;

	ARTS_LevelEnd* m_LevelEnd;
	ARTS_PlayerSpawner* m_PlayerSpawner;
	ARTS_LevelBounds* m_LevelBounds;
	TArray<AUpgradeShopBase*> m_Shops;

};
