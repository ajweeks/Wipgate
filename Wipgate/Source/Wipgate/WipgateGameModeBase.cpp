// Fill out your copyright notice in the Description page of Project Settings.

#include "WipgateGameModeBase.h"
#include "Engine/DataTable.h"
#include "RTS_Team.h"
#include "RTS_GameState.h"
#include "RTS_Entity.h"
#include "RTS_PlayerController.h"
#include "RTS_EntitySpawnerBase.h"
#include "RTS_GameInstance.h"
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
		auto entitySpawn = Cast<ARTS_EntitySpawnerBase>(actor);
		entitySpawn->SpawnEntities();
	}

	//Calculate upgrades
	auto gameinstance = Cast<URTS_GameInstance>(GetGameInstance());
	if (gameinstance)
	{
		playercontroller->Team->Upgrades.Empty();
		playercontroller->Team->AddUpgrades(gameinstance->ActiveUpgrades);
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
