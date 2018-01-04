// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTS_EntitySpawnerBase.h"
#include "Helpers/EntityHelpers.h"
#include "RTS_PlayerSpawner.generated.h"

/**
 * 
 */
UCLASS()
class WIPGATE_API ARTS_PlayerSpawner : public ARTS_EntitySpawnerBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FEntitySave> SavedEntities;
};
