// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Helpers/EntityHelpers.h"
#include "RTS_EntitySpawner.generated.h"

UCLASS()
class WIPGATE_API ARTS_EntitySpawner : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ARTS_EntitySpawner();
	
	UFUNCTION(BlueprintImplementableEvent)
		void SpawnEntities();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FEntitySpawn> EntitySpawns;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float Radius;
};
