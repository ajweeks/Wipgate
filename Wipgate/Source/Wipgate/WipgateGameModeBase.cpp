// Fill out your copyright notice in the Description page of Project Settings.

#include "WipgateGameModeBase.h"
#include "Engine/DataTable.h"
#include "RTS_Team.h"
#include "RTS_GameState.h"

DEFINE_LOG_CATEGORY(WipgateGameModeBase);

void AWipgateGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	if (!m_Table)
		return;

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
			team->Name = name;
			team->Alignment = row->Alignment;
			team->Color = row->Color;

			gamestate->Teams.Add(team);
		}

	}
}

