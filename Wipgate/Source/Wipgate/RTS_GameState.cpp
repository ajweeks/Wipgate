// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_GameState.h"

#include "RTS_Entity.h"

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