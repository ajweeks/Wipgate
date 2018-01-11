// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_EntitySpawnerBase.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "WipgateGameModeBase.h"
#include "RTS_Entity.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(RTS_EntitySpawnerBase);

// Sets default values
ARTS_EntitySpawnerBase::ARTS_EntitySpawnerBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ARTS_EntitySpawnerBase::InitializeEntity(ARTS_Entity * entity, ETeamAlignment alignment)
{
	auto gamemode = GetWorld()->GetAuthGameMode<AWipgateGameModeBase>();
	if (gamemode)
	{
		URTS_Team* team = gamemode->GetTeamWithAlignment(alignment);
		entity->Team = team;
		team->Entities.Add(entity);
		entity->Alignment = alignment;
		entity->PostInitialize();
		auto gamestate = gamemode->GetGameState<ARTS_GameState>();
		if (gamestate)
		{
			gamestate->Entities.Add(entity);
		}
		else
		{
			UE_LOG(WipgateGameModeBase, Error, TEXT("InitializeEntity > No gamestate found!"));
		}
	}
	else
	{
		UE_LOG(WipgateGameModeBase, Error, TEXT("InitializeEntity > No gamemode found!"));
	}
}

void ARTS_EntitySpawnerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FColor color = FColor::Red;
	if (m_IsActive)
		color = FColor::Green;
	DrawDebugCircle(GetWorld(), GetActorLocation(), Radius, 36, color, false, 0.f, (uint8)'\000', 3.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);
}

bool ARTS_EntitySpawnerBase::ShouldTickIfViewportsOnly() const
{
	return true;
}
