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
#include "UpgradeShopBase.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "JsonObjectConverter.h"

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

	if (UseFriendlyAddedTroops && !m_FriendlyAddedTroops)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No friendly added troops table was linked. Returning..."));
		return;
	}

	ARTS_GameState* gamestate = GetGameState<ARTS_GameState>();
	URTS_GameInstance* gameinstance = Cast<URTS_GameInstance>(GetGameInstance());

	ARTS_PlayerController* playercontroller = Cast<ARTS_PlayerController>(GetWorld()->GetFirstPlayerController());
	if (!playercontroller)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No playercontroller. Returning..."));
		return;
	}
	SelectRandomLevelSetup();
	playercontroller->Initialize();

	// Get table rows
	TArray<FTeamRow*> rows;
	m_TeamTable->GetAllRows("BeginPlay > Team table not found!", rows);
	TArray<FName> rowNames = m_TeamTable->GetRowNames();

	if (m_TeamTable->GetRowNames().Num() <= 0)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No teams in team datatable. Returning..."));
		return;
	}

	// Loop over all team rows and make team objects if it's the first round
	if (gameinstance->Teams.Num() == 0)
	{
		for (int i = 0; i < rows.Num(); ++i)
		{
			//Take random json
			if (m_UseJSON && UnitToSpawnPath.Num() > 0)
			{
				gameinstance->RoundAdditionIndex = FMath::RandRange(0, UnitToSpawnPath.Num() - 1);
			}
			else if (m_UseJSON)
			{
				gameinstance->RoundAdditionIndex = -1;
				UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No JSON files linked!"));
			}

			//Make teams
			FTeamRow* row = rows[i];
			if (row)
			{
				FName name = rowNames[i];

				// Create team
				URTS_Team* team = NewObject<URTS_Team>();
				team->SetAlignment(row->Alignment);
				team->Color = row->Color;
				team->GameInstance = gameinstance;

				gameinstance->Teams.Add(team);
			}
		}
	}

	// Set playercontroller team if it's player
	playercontroller->Team = GetTeamWithAlignment(ETeamAlignment::E_PLAYER);

	//Clear entities in team
	for (auto team : gameinstance->Teams)
	{
		team->Entities.Empty();
	}

	// Check users in that group
	for (ARTS_Entity* entity : gamestate->Entities)
	{
		entity->Team = GetTeamWithAlignment(entity->Alignment);
		if(entity->Team)
			entity->Team->Entities.Add(entity);
	}

	// Check if the playercontroller has a team
	if (!playercontroller->Team)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > Playercontroller has no team!"));
		return;
	}

	// Check if there are still null teams
	for (ARTS_Entity* entity : gamestate->Entities)
	{
		if (entity->Team == nullptr)
		{
			UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > %s does not have a team, attempting to assign default"), *entity->GetHumanReadableName());
			if (gameinstance->Teams.Num() > 0)
			{
				entity->Team = gameinstance->Teams[0];
				gameinstance->Teams[0]->Entities.Add(entity);
			}
			else
			{
				UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No team present"));
				break;
			}
		}
		entity->PostInitialize();
	}

		// Spawn entities
		TArray<AActor*> actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_EntitySpawner::StaticClass(), actors);
		for (auto actor : actors)
		{
			ARTS_EntitySpawner* entitySpawn = Cast<ARTS_EntitySpawner>(actor);

			// Percentual chance of spawning
			float rand = FMath::FRandRange(0, 1);
			if (rand < BaseSpawnChance + (gameinstance->CurrentRound * SpawnChanceRoundIncrease) + entitySpawn->SpawnModifier)
			{
				entitySpawn->SpawnEntities();
			}
		}
		actors.Empty();

		if (UseFriendlyAddedTroops)
		{
			// Add entities to player's team
			TArray<FEntityRow*> playerRows;
			FString json;
			FString spawnPath = "";

			if (gameinstance->RoundAdditionIndex >= 0)
			{
				spawnPath = UnitToSpawnPath[gameinstance->RoundAdditionIndex];
			}
			FString path = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + spawnPath;
			auto success = FFileHelper::LoadFileToString(json, *path, FFileHelper::EHashOptions::EnableVerify);


			//TODO: Add check for development build
			if (m_UseJSON && success)
			{
				//Read json file
				TArray<TSharedPtr<FJsonValue>> arrayDeserialized;
				TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(json);
				if (FJsonSerializer::Deserialize(reader, arrayDeserialized))
				{
					for (TSharedPtr<FJsonValue> jsonObject : arrayDeserialized)
					{
						TArray<FEntitySpawn> spawns;
						auto row = jsonObject->AsObject();
						auto round = row->GetIntegerField("Round");
						auto spawnsJson = row->GetArrayField("Spawns");
						FJsonObjectConverter::JsonArrayToUStruct(spawnsJson, &spawns, 0, 0);
						if (round == gameinstance->CurrentRound)
						{
							for (int i = 0; i < spawns.Num(); i++)
							{
								FEntitySpawn es = spawns[i];
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
				}
				else
				{
					UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > Could not deserialize json file!"));
				}
			}
			else if (!m_UseJSON)
			{
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
			}
			else
			{
				UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > %s path not found!"), *path);
			}
		}

		// Spawn player
		auto playerspawner = GetPlayerSpawner();
		if (playerspawner)
			playerspawner->SpawnEntities();

		for (auto team : gameinstance->Teams)
		{
			//Recalculate upgrades
			team->CalculateUpgradeEffects();
		}

		// Give random upgrade to enemy team
		auto team = GetTeamWithAlignment(ETeamAlignment::E_AGGRESSIVE_AI);
		if (team)
		{
			// Add random upgrade
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
		else
		{
			UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No enemy team. Returning..."));
		}

		// Update luma
		playercontroller->AddLuma(gameinstance->CurrentLuma);
}

URTS_Team* AWipgateGameModeBase::GetTeamWithAlignment(ETeamAlignment alignment)
{
	URTS_GameInstance* gameinstance = Cast<URTS_GameInstance>(GetGameInstance());
	for (URTS_Team* team : gameinstance->Teams)
	{
		if (team->GetAlignment() == alignment)
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

	if (!gameinstance)
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

	UWorld* world = GetWorld();
	FString currentLevelName = UGameplayStatics::GetCurrentLevelName(world);
	// Open same level
	if (currentLevelName.Equals("Gameplay_Layout_Deco_02"))
	{
		UGameplayStatics::OpenLevel(world, "Gameplay_Layout_Deco_04");
	}
	else if (currentLevelName.Equals("Gameplay_Layout_Deco_04"))
	{
		UGameplayStatics::OpenLevel(world, "Gameplay_Layout_Deco_02");
	}
	else if (currentLevelName.Equals("TutorialMap_01") ||
		currentLevelName.Equals("TutorialMap_02"))
	{
		UGameplayStatics::OpenLevel(world, "MainMenu");
	}
	else
	{
		UGameplayStatics::OpenLevel(world, *UGameplayStatics::GetCurrentLevelName(world));
	}

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
	return m_LevelEnd;
}

void AWipgateGameModeBase::SelectRandomLevelSetup()
{
	//Get all spawns
	TArray<AActor*> uncastedSpawners;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_PlayerSpawner::StaticClass(), uncastedSpawners);

	if (uncastedSpawners.Num() > 0)
	{
		TArray<ARTS_PlayerSpawner*> spawners;

		//Convert spawners that have an endgoal
		for (auto uncastedSpawner : uncastedSpawners)
		{
			ARTS_PlayerSpawner* castedSpawner = Cast<ARTS_PlayerSpawner>(uncastedSpawner);
			if (!castedSpawner)
				continue;

			if (castedSpawner->LevelEnd)
			{
				spawners.Add(castedSpawner);
			}
			else
			{
				UE_LOG(WipgateGameModeBase, Warning, TEXT("SelectRandomLevelSetup > %s does not have a levelend linked to it."), *castedSpawner->GetName());
			}
		}

		if (spawners.Num() <= 0)
		{
			UE_LOG(WipgateGameModeBase, Error, TEXT("SelectRandomLevelSetup > No player spawners with levelends available. Returning..."));
			return;
		}

		//Pick a random spawner
		int chosenIndex = FMath::RandRange(0, spawners.Num() - 1);
		m_LevelEnd = spawners[chosenIndex]->LevelEnd;

		//Entity spawner at the end should have 100% spawn rate
		if (m_LevelEnd->EntitySpawner)
			m_LevelEnd->EntitySpawner->SpawnModifier = 1.f;

		m_Shop = spawners[chosenIndex]->Shop;
		m_PlayerSpawner = spawners[chosenIndex];

		//Destroy all endzones that weren't selected
		TArray<AActor*> uncastedDeletionObjects;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_LevelEnd::StaticClass(), uncastedDeletionObjects);
		for (int32 i = uncastedDeletionObjects.Num() - 1; i >= 0; --i)
		{
			if (uncastedDeletionObjects[i] && uncastedDeletionObjects[i] != m_LevelEnd)
			{
				uncastedDeletionObjects[i]->Destroy();
				auto end = Cast<ARTS_LevelEnd>(uncastedDeletionObjects[i]);
				if(end && end->EntitySpawner)
					end->EntitySpawner->Destroy();
			}
		}
		uncastedDeletionObjects.Empty();

		//Destroy all shops that weren't selected
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUpgradeShopBase::StaticClass(), uncastedDeletionObjects);
		for (int32 i = uncastedDeletionObjects.Num() - 1; i >= 0; --i)
		{
			if (uncastedDeletionObjects[i] != m_Shop)
			{
				uncastedDeletionObjects[i]->Destroy();
			}
		}
		uncastedDeletionObjects.Empty();

		//Destroy all spawners that weren't selected
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_PlayerSpawner::StaticClass(), uncastedDeletionObjects);
		for (int32 i = uncastedDeletionObjects.Num() - 1; i >= 0; --i)
		{
			if (uncastedDeletionObjects[i] != m_PlayerSpawner)
			{
				uncastedDeletionObjects[i]->Destroy();
			}
		}
	}
	else
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("No level end found in map!"));
	}
}

ARTS_LevelBounds* AWipgateGameModeBase::GetLevelBounds()
{
	if (m_LevelBounds)
	{
		return m_LevelBounds;
	}

	TArray<AActor*> levelBoundsObjects;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_LevelBounds::StaticClass(), levelBoundsObjects);

	if (levelBoundsObjects.Num() >= 1)
	{
		m_LevelBounds = Cast<ARTS_LevelBounds>(levelBoundsObjects[0]);
	}
	else
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("No level bounds found in map!"));
	}

	return m_LevelBounds;
}
