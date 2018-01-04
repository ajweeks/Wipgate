// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Helpers/EntityHelpers.h"
#include "RTS_GameState.h"
#include "RTS_EntitySpawnerBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(RTS_EntitySpawnerBase, Log, All);

UCLASS()
class WIPGATE_API ARTS_EntitySpawnerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARTS_EntitySpawnerBase();

	/* FUNCTIONS */
	UFUNCTION(BlueprintImplementableEvent)
		void SpawnEntities();
	UFUNCTION(BlueprintCallable)
		void InitializeEntity(ARTS_Entity* entity, ETeamAlignment alignment);
	virtual void Tick(float DeltaTime) override;

	/* PROPERTIES */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float Radius = 250.f;
	
};
