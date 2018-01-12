// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_EntitySpawner.h"
#include "RTS_Entity.h"
#include "RTS_EntitySpawnerBase.h"


// Sets default values
ARTS_EntitySpawner::ARTS_EntitySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ARTS_EntitySpawner::InitializeEntity(ARTS_Entity * entity, ETeamAlignment teamAlignment)
{
	ARTS_EntitySpawnerBase::InitializeEntity(entity, teamAlignment);
	entity->Spawner = this;
}
