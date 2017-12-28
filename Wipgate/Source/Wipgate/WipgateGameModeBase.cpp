// Fill out your copyright notice in the Description page of Project Settings.

#include "WipgateGameModeBase.h"
#include "Engine/DataTable.h"
#include "RTS_Team.h"
#include "RTS_GameState.h"
#include "RTS_Entity.h"
#include "RTS_PlayerController.h"

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

			//Set playercontroller team if it's player
			if (team->Alignment == ETeamAlignment::E_PLAYER)
			{
				Cast<ARTS_PlayerController>(GetWorld()->GetFirstPlayerController())->Team = team;
			}

			//Check users in that group
			for (ARTS_Entity* entity : gamestate->Entities)
			{
				if (entity->Alignment == team->Alignment)
				{
					entity->SetTeam(team);
					team->Entities.Add(entity);
				}
			}
		}
	}

	//Check if the playercontroller has a team
	if (!Cast<ARTS_PlayerController>(GetWorld()->GetFirstPlayerController())->Team)
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
				entity->SetTeam(gamestate->Teams[0]);
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
}

