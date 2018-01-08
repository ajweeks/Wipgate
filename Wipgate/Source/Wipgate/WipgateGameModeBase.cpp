// Fill out your copyright notice in the Description page of Project Settings.

#include "WipgateGameModeBase.h"
#include "Engine/DataTable.h"
#include "RTS_Team.h"
#include "RTS_GameState.h"
#include "RTS_Entity.h"
#include "RTS_PlayerController.h"
#include "RTS_EntitySpawner.h"
#include "RTS_PlayerSpawner.h"
#include "RTS_GameInstance.h"
#include "RTS_LevelEnd.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

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

	//Check if there are still nullpts
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

		//Spawn players
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_PlayerSpawner::StaticClass(), actors);
		if (actors.Num() <= 0)
		{
			UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No playerspawn available!"));
		}
		else 
		{
			if (actors.Num() > 1)
			{
				UE_LOG(WipgateGameModeBase, Warning, TEXT("BeginPlay > More than 1 playerspawn available, using first one.!"));
			}
			Cast<ARTS_PlayerSpawner>(actors[0])->SpawnEntities();
		}

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

		//Update currency & luma
		playercontroller->AddCurrency(gameinstance->CurrentCurrency);
		playercontroller->AddLuma(gameinstance->CurrentLuma);
	}
	else
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No game instance found!"));
	}

	//Get all endzones
	TArray<AActor*> levelends;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_LevelEnd::StaticClass(), levelends);

	if (levelends.Num() > 0)
	{
		//Get potential end zone
		int index = FMath::RandRange(0, levelends.Num() - 1);

		//Destroy any endzone that wasn't selected
		for (int i = levelends.Num() - 1; i >= 0; --i)
		{
			if (i == index)
				continue;
			levelends[i]->Destroy();
		}
	}
	else
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No level end found!"));
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

	gameinstance->CurrentCurrency = playercontroller->GetCurrentCurrencyAmount();
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
