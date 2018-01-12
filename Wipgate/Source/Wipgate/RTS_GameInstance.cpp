// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_GameInstance.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "RTS_GameState.h"
#include "RTS_Entity.h"
#include "Helpers/EntityHelpers.h"

DEFINE_LOG_CATEGORY(RTS_GameInstance);

void URTS_GameInstance::AddLuma(int amount)
{
	CurrentLuma += amount;
}

void URTS_GameInstance::SpendLuma(int amount)
{
	CurrentLuma -= amount;
}

void URTS_GameInstance::EndRound()
{
	SavedEntities.Empty();
	auto gamestate = GetWorld()->GetGameState<ARTS_GameState>();
	if (gamestate)
	{
		CurrentRound++;
		SavedEntities.Empty();
		for (auto entity : gamestate->Entities)
		{
			if (!entity)
				continue;

			if (entity->Alignment == ETeamAlignment::E_PLAYER && entity->EntityType != EEntityType::E_STRUCTURE)
			{
				FEntitySave save;
				save.Entity = entity->GetClass();
				save.Health = entity->Health;
				save.LumaStats = entity->CurrentLumaStats;
				SavedEntities.Add(save);
			}
		}
	}
	else
	{
		UE_LOG(RTS_GameInstance, Error, TEXT("EndRound > No gamestate present!"));

	}

}
