// Fill out your copyright notice in the Description page of Project Settings.

#include "WipgateGameModeBase.h"
#include "Engine/DataTable.h"
#include "RTS_Team.h"
#include "RTS_GameState.h"
#include "RTS_Entity.h"

DEFINE_LOG_CATEGORY(WipgateGameModeBase);

void AWipgateGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	if (!m_Table)
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("WipgateGameModeBase::BeginPlay > No table was linked. Returning..."));
		return;
	}

	TArray<FTeamRow*> rows;
	m_Table->GetAllRows("WipgateGameModeBase::BeginPlay() > Table not found!", rows);
	TArray<FName> rowNames = m_Table->GetRowNames();

	ARTS_GameState* gamestate = GetGameState<ARTS_GameState>();
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

			gamestate->Teams.Add(team);

			//Check users in that group
			for (ARTS_Entity* entity : gamestate->Entities)
			{
				if (entity->Alignment == team->Alignment)
				{
					entity->SetTeam(team);
				}
			}
		}
	}

	//Check if there are still nullpts
	for (ARTS_Entity* entity : gamestate->Entities)
	{
		if (entity->Team == nullptr)
		{
			UE_LOG(WipgateGameModeBase, Error, TEXT("WipgateGameModeBase::BeginPlay > Unit does not have a team, attempting to assign default"));
			if (gamestate->Teams.Num() > 0)
			{
				entity->SetTeam(gamestate->Teams[0]);
			}
			else
			{
				UE_LOG(WipgateGameModeBase, Error, TEXT("WipgateGameModeBase::BeginPlay > No team present"));
				break;
			}
		}
		if (entity->Alignment != ETeamAlignment::E_PLAYER)
		{
			UE_LOG(WipgateGameModeBase, Error, TEXT("WipgateGameModeBase::BeginPlay > Unit does not have a team, attempting to assign default"));
			auto i = 0;
			i -= 1;
		}
		entity->PostInitialize();
	}
}

