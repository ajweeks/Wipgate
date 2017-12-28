// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_GameState.h"

#include "RTS_Entity.h"
#include "RTS_Unit.h"

DEFINE_LOG_CATEGORY(RTS_Gamestate_log);

void ARTS_GameState::RemoveEntity(ARTS_Entity* Entity)
{
	if(Entity)
	{
		Entity->SetSelected(false);
		Entities.Remove(Entity);
		SelectedEntities.Remove(Entity);

		UE_LOG(LogTemp, Display, TEXT("ARTS_GameState::RemoveUnit > Removed unit from Units array. %i remaining."), Entities.Num());
	}
}

float ARTS_GameState::GetPercentOfFriendlyUnitsInLevelGoal()
{
	int32 countInGoal = 0;
	int32 countRequired = 0;

	for (int32 i = 0; i < Entities.Num(); ++i)
	{
		ARTS_Unit* unit = Cast<ARTS_Unit>(Entities[i]);
		if (unit &&
			unit->Team->Alignment == ETeamAlignment::E_PLAYER &&
			unit->Health > 0)
		{
			++countRequired;

			if (unit->InLevelGoal)
			{
				++countInGoal;
			}
		}
	}

	return (float)countInGoal / (float)countRequired;
}
