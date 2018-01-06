// Fill out your copyright notice in the Description page of Project Settings.

#include "WipgateGameModeBase.h"
#include "Engine/DataTable.h"
#include "RTS_Team.h"
#include "RTS_GameState.h"
#include "RTS_Entity.h"
#include "RTS_PlayerController.h"
#include "RTS_EntitySpawnerBase.h"
#include "RTS_GameInstance.h"
#include "RTS_LevelEnd.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(WipgateGameModeBase);

void AWipgateGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	if (!m_Table)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("BeginPlay > No table was linked. Returning..."));
		return;
	}

	//Get table rows
	TArray<FTeamRow*> rows;
	m_Table->GetAllRows("BeginPlay > Table not found!", rows);
	TArray<FName> rowNames = m_Table->GetRowNames();

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

	//Spawn actors
	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_EntitySpawnerBase::StaticClass(), actors);
	for (auto actor : actors)
	{
		ARTS_EntitySpawnerBase* entitySpawn = Cast<ARTS_EntitySpawnerBase>(actor);
		entitySpawn->SpawnEntities();
	}

	URTS_GameInstance* gameinstance = Cast<URTS_GameInstance>(GetGameInstance());
	if (gameinstance)
	{
		//Calculate upgrades
		playercontroller->Team->Upgrades.Empty();
		playercontroller->Team->AddUpgrades(gameinstance->ActiveUpgrades);

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
		UE_LOG(WipgateGameModeBase, Error, TEXT("SaveCurrency > No playercontroller. Returning..."));
		return;
	}

	if(!gameinstance)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("SaveCurrency > No gameinstance. Returning..."));
		return;
	}

	gameinstance->CurrentCurrency = playercontroller->GetCurrentCurrencyAmount();
	gameinstance->CurrentLuma = playercontroller->GetCurrentLumaAmount();
}

void AWipgateGameModeBase::NextLevel()
{
	ARTS_PlayerController* playercontroller = Cast<ARTS_PlayerController>(GetWorld()->GetFirstPlayerController());
	if (!playercontroller)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("SaveCurrency > No playercontroller. Returning..."));
		return;
	}
	SaveResources();
	UGameplayStatics::OpenLevel(GetWorld(), FName(*UGameplayStatics::GetCurrentLevelName(GetWorld())));
}
