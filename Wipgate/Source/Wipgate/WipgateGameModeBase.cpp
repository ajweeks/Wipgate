// Fill out your copyright notice in the Description page of Project Settings.

#include "WipgateGameModeBase.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

#include "RTS_Team.h"
#include "RTS_GameState.h"
#include "RTS_Entity.h"
#include "RTS_PlayerController.h"
#include "RTS_EntitySpawner.h"
#include "RTS_PlayerSpawner.h"
#include "RTS_GameInstance.h"
#include "RTS_LevelEnd.h"
#include "RTS_PlayerSpawner.h"
#include "RTS_LevelBounds.h"

DEFINE_LOG_CATEGORY(WipgateGameModeBase);

void AWipgateGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	if (!m_TeamTable)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No team table was linked. Returning..."));
		return;
	}

	if (!m_EnemyUpgradeTable)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No enemy upgrade table was linked. Returning..."));
		return;
	}

	if (!m_FriendlyAddedTroops)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No friendly added troops table was linked. Returning..."));
		return;
	}

	//Get table rows
	TArray<FTeamRow*> rows;
	m_TeamTable->GetAllRows("BeginPlay > Team table not found!", rows);
	TArray<FName> rowNames = m_TeamTable->GetRowNames();

	ARTS_GameState* gamestate = GetGameState<ARTS_GameState>();
	ARTS_PlayerController* playercontroller = Cast<ARTS_PlayerController>(GetWorld()->GetFirstPlayerController());
	if (!playercontroller)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No playercontroller. Returning..."));
		return;
	}

	//Loop over all team rows and make team objects
	for (int i = 0; i < rows.Num(); ++i)
	{
		FTeamRow* row = rows[i];
		if (row)
		{
			FName name = rowNames[i];

			//Create team
			URTS_Team* team = NewObject<URTS_Team>();
			team->Alignment = row->Alignment;
			team->Color = row->Color;
			team->World = GetWorld();

			gamestate->Teams.Add(team);

			//Set playercontroller team if it's player
			if (team->Alignment == ETeamAlignment::E_PLAYER)
			{
				playercontroller->Team = team;
			}

			//Check users in that group
			for (ARTS_Entity* entity : gamestate->Entities)
			{
				if (entity->Alignment == team->Alignment)
				{
					entity->Team = team;
					team->Entities.Add(entity);
				}
			}
		}
	}

	//Check if the playercontroller has a team
	if (!playercontroller->Team)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > Playercontroller has no team!"));
		return;
	}

	//Check if there are still null teams
	for (ARTS_Entity* entity : gamestate->Entities)
	{
		if (entity->Team == nullptr)
		{
			UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > %s does not have a team, attempting to assign default"), *entity->GetHumanReadableName());
			if (gamestate->Teams.Num() > 0)
			{
				entity->Team = gamestate->Teams[0];
				gamestate->Teams[0]->Entities.Add(entity);
			}
			else
			{
				UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No team present"));
				break;
			}
		}
		entity->PostInitialize();
	}

	URTS_GameInstance* gameinstance = Cast<URTS_GameInstance>(GetGameInstance());
	if (gameinstance)
		{
		//Spawn entities
		TArray<AActor*> actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_EntitySpawner::StaticClass(), actors);
		for (auto actor : actors)
		{
			ARTS_EntitySpawner* entitySpawn = Cast<ARTS_EntitySpawner>(actor);

			//Percentual chance of spawning
			float rand = FMath::FRandRange(0, 1);
			if (rand < BaseSpawnChance + (gameinstance->CurrentRound * SpawnChanceRoundIncrease) + entitySpawn->SpawnModifier)
			{
				entitySpawn->SpawnEntities();
			}
		}
		actors.Empty();

		//Add entities to player's team
		TArray<FEntityRow*> playerRows;
		m_FriendlyAddedTroops->GetAllRows("BeginPlay > Added friendly troops table not found!", playerRows);
		
		for (auto pr : playerRows)
		{
			if (pr->Round == gameinstance->CurrentRound)
			{
				for (int i = 0; i < pr->Spawns.Num(); i++)
				{
					FEntitySpawn es = pr->Spawns[i];
					ARTS_Entity* defaultObject = es.Entity.GetDefaultObject();
					for (int i = 0; i < es.Amount; i++)
					{
						FEntitySave save;
						save.Entity = es.Entity;
						save.Health = defaultObject->BaseDefenceStats.MaxHealth;
						save.LumaStats = defaultObject->CurrentLumaStats;
						gameinstance->SavedEntities.Add(save);
					}
				}
			}
		}


		//Spawn player
		auto playerspawner = GetPlayerSpawner();
		if(playerspawner)
			playerspawner->SpawnEntities();

		for (auto team : gamestate->Teams)
		{
			//Empty all upgrades
			team->Upgrades.Empty();
		}

		//Add upgrades to player
		playercontroller->Team->AddUpgrades(gameinstance->ActiveUpgrades);

		//Give random upgrades to enemy based on round amount
		auto team = GetTeamWithAlignment(ETeamAlignment::E_AGGRESSIVE_AI);
		if (team)
		{
			auto round = gameinstance->CurrentRound;
			for (int i = 0; i < round; ++i)
			{
				//Add random upgrade
				TArray<FEnemyUpgradeRow*> rows;
				m_EnemyUpgradeTable->GetAllRows("BeginPlay > Enemy upgrade table not found!", rows);
				if (rows.Num() > 0)
				{
					int randomUpgradeIndex = FMath::RandRange(0, rows.Num() - 1);
					auto row = rows[randomUpgradeIndex];
					team->AddUpgrades(row->Upgrades);
				}
				else
				{
					UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No enemy upgrades in enemy upgrade datatable!"));
				}
			}
		}
		else
		{
			UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No enemy team. Returning..."));
		}

		//Update luma
		playercontroller->AddLuma(gameinstance->CurrentLuma);
	}
	else
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No game instance found!"));
	}
}

URTS_Team * AWipgateGameModeBase::GetTeamWithAlignment(ETeamAlignment alignment)
{
	ARTS_GameState* gamestate = GetGameState<ARTS_GameState>();
	for (URTS_Team* team : gamestate->Teams)
	{
		if (team->Alignment == alignment)
		{
			return team;
		}
	}

	UE_LOG(WipgateGameModeBase, Error, TEXT("GetTeamWithAlignment > No team was found with that alignment"));
	return nullptr;
}

void AWipgateGameModeBase::SaveResources()
{
	ARTS_PlayerController* playercontroller = Cast<ARTS_PlayerController>(GetWorld()->GetFirstPlayerController());
	URTS_GameInstance* gameinstance = Cast<URTS_GameInstance>(GetGameInstance());
	if (!playercontroller)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("SaveResources > No playercontroller. Returning..."));
		return;
	}

	if(!gameinstance)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("SaveResources > No gameinstance. Returning..."));
		return;
	}

	gameinstance->CurrentLuma = playercontroller->GetCurrentLumaAmount();
}

void AWipgateGameModeBase::NextLevel()
{
	ARTS_PlayerController* playercontroller = Cast<ARTS_PlayerController>(GetWorld()->GetFirstPlayerController());
	URTS_GameInstance* gameinstance = Cast<URTS_GameInstance>(GetGameInstance());
	if (!playercontroller)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("NextLevel > No playercontroller. Returning..."));
		return;
	}
	if (!gameinstance)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("NextLevel > No gameinstance. Returning..."));
		return;
	}
	SaveResources();

	//Open same level
	UGameplayStatics::OpenLevel(GetWorld(), FName(*UGameplayStatics::GetCurrentLevelName(GetWorld())));
}

ARTS_PlayerSpawner* AWipgateGameModeBase::GetPlayerSpawner()
{
	if (m_PlayerSpawner)
	{
		return m_PlayerSpawner;
	}

	TArray<AActor*> playerSpawners;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_PlayerSpawner::StaticClass(), playerSpawners);

	if (playerSpawners.Num() >= 1)
	{
		m_PlayerSpawner = Cast<ARTS_PlayerSpawner>(playerSpawners[0]);
		return m_PlayerSpawner;
	}
	else
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("No player spawner found in map!"));
		return nullptr;
	}
}

ARTS_LevelEnd* AWipgateGameModeBase::GetLevelEnd()
{
	if (m_LevelEnd)
	{
		return m_LevelEnd;
	}

	TArray<AActor*> levelEnds;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_LevelEnd::StaticClass(), levelEnds);

	if (levelEnds.Num() > 0)
	{
		//Get potential end zone
		int chosenEndZoneIndex = FMath::RandRange(0, levelEnds.Num() - 1);

		//Destroy any endzone that wasn't selected
		for (int i = levelEnds.Num() - 1; i >= 0; --i)
		{
			if (i == chosenEndZoneIndex)
			{
				m_LevelEnd = Cast<ARTS_LevelEnd>(levelEnds[i]);
			}
			else
			{
				levelEnds[i]->Destroy();
			}
		}
	}
	else
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("No level end found in map!"));
	}

	return m_LevelEnd;
}

ARTS_LevelBounds* AWipgateGameModeBase::GetLevelBounds()
{
	if (m_LevelBounds)
	{
		return m_LevelBounds;
	}

	TArray<AActor*> levelBoundsObjects;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_LevelBounds::StaticClass(), levelBoundsObjects);

	if (levelBoundsObjects.Num() >=	1)
	{
		m_LevelBounds = Cast<ARTS_LevelBounds>(levelBoundsObjects[0]);
	}
	else
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("No level bounds found in map!"));
	}

	return m_LevelBounds;
}
