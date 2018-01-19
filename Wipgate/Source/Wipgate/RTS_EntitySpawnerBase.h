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
		virtual void InitializeEntity(ARTS_Entity* entity, ETeamAlignment teamAlignment);
	UFUNCTION(BlueprintCallable)
		void AddEntity(ARTS_Entity* entity);
	UFUNCTION(BlueprintCallable)
		void RemoveEntity(ARTS_Entity* entity);
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

	/* PROPERTIES */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Radius = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int SpawnAttempts = 3;

protected:
	//Determines the color of the debug circle
	UPROPERTY(BlueprintReadWrite)
	bool m_IsActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<ARTS_Entity*> m_SpawnedEntities;
	
};
