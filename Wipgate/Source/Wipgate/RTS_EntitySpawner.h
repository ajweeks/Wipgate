// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RTS_EntitySpawnerBase.h"
#include "RTS_EntitySpawner.generated.h"

UCLASS()
class WIPGATE_API ARTS_EntitySpawner : public ARTS_EntitySpawnerBase
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ARTS_EntitySpawner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEntitySpawn> EntitySpawns;


	//Modifies the spawn rate on top of the spawn rate defined in the gamemode
	UPROPERTY(EditAnywhere, meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
		float SpawnModifier = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETeamAlignment Alignment = ETeamAlignment::E_AGGRESSIVE_AI;
};
