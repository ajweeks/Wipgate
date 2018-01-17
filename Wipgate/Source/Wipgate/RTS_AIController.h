#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RTS_Entity.h"
#include "Command.h"

#include "RTS_AIController.generated.h"

UENUM(BlueprintType)
enum class EUNIT_TASK : uint8
{
	IDLE 			UMETA(DisplayName = "Idle"),			// No task
	MOVING 			UMETA(DisplayName = "Moving"),			// Moving toward a target location
	ATTACK_MOVING	UMETA(DisplayName = "Attack moving"),	// Moving toward a target location, will switch to attacking when enemy comes in range along the way
	CHASING			UMETA(DisplayName = "Chasing"),			// Moving toward a unit to attack
	FOLLOWING		UMETA(DisplayName = "Following"),		// Staying near an ally
	ATTACKING		UMETA(DisplayName = "Attacking"),		// Attacking an enemy
	CASTING			UMETA(DisplayName = "Casting "),		// Warming up ability (pre-execute), CAN be cancelled
	EXECUTING		UMETA(DisplayName = "Executing"),		// Executing ability, not controllable by the player, can NOT be cancelled
};

UCLASS()
class WIPGATE_API ARTS_AIController : public AAIController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Setters")
		void SetTargetLocation(const FVector target);
	UFUNCTION(BlueprintCallable, Category = "Setters")
		void SetTargetEntity(ARTS_Entity* targetEntity);
	UFUNCTION(BlueprintCallable, Category = "Setters")
		void SetCurrentTask(const EUNIT_TASK task);

	UFUNCTION(BlueprintPure, Category = "Getters")
		UCommand* GetCurrentCommand() { return m_CurrentCommand; }
	UFUNCTION(BlueprintPure, Category = "Getters")
		EUNIT_TASK GetCurrentTask() { return m_CurrentTask; }
	UFUNCTION(BlueprintPure, Category = "Getters")
		bool IsAlert() { return m_IsAlert; }
	UFUNCTION(BlueprintPure, Category = "Getters")
		ARTS_AIController* GetController(ARTS_Entity* entity);


	/* --- AI functions --- */
	UFUNCTION(BlueprintCallable, Category = "AI")
		TArray<ARTS_Entity*> GetEntitiesWithTask(const TArray<ARTS_Entity*> entities, const EUNIT_TASK task);
	UFUNCTION(BlueprintCallable, Category = "AI")
		ARTS_Entity* GetClosestEntity(const TArray<ARTS_Entity*> entities);
	UFUNCTION(BlueprintCallable, Category = "AI")
		ERelativeAlignment GetRelativeAlignment(const ARTS_Entity* a, const ARTS_Entity* b);
	UFUNCTION(BlueprintCallable, Category = "AI")
		TArray<ARTS_Entity*> GetEnemiesInAttackRange();
	UFUNCTION(BlueprintCallable, Category = "AI")
		TArray<ARTS_Entity*> GetEnemiesInVisionRange();
	UFUNCTION(BlueprintCallable, Category = "AI")
		bool IsTargetAttacking();
	UFUNCTION(BlueprintCallable, Category = "AI")
		bool IsTargetAgressive();
	UFUNCTION(BlueprintCallable, Category = "AI")
		TArray<ARTS_Entity*> GetFriendlyEntities(const TArray<ARTS_Entity*> entities);
	UFUNCTION(BlueprintCallable, Category = "AI")
		TArray<FVector> GetEntityPositions(const TArray<ARTS_Entity*> entities);

	UFUNCTION(BlueprintCallable, Category = "AI")
		void RotateTowardsTarget();
	UFUNCTION(BlueprintCallable, Category = "AI")
		void RotateTowardsTargetLocation();

	/* --- Flocking functions --- */
	UFUNCTION(BlueprintCallable, Category = "Flocking")
		void StoreNearbyEntities(const float radius);
	UFUNCTION(BlueprintCallable, Category = "Flocking")
		FVector GetSeparationVector(const float radius);
	UFUNCTION(BlueprintCallable, Category = "Flocking")
		FVector GetCohesionVector();
	UFUNCTION(BlueprintCallable, Category = "Flocking")
		FVector GetSeekVector(const FVector target, FVector& nextPathPoint);
	UFUNCTION(BlueprintCallable, Category = "Flocking")
		FVector GetAvoidanceVector(const FVector nextPathPoint);
	UFUNCTION(BlueprintCallable, Category = "Flocking")
		TArray<ARTS_Entity*> GetObstacles();
	UFUNCTION(BlueprintCallable, Category = "Flocking")
		void RenderFlockingDebug(const FVector separation, const FVector cohesion,
			const FVector seek, const FVector avoidance, float stepLength);
	UFUNCTION(BlueprintCallable, Category = "Flocking")
		bool FlockMoveToLocation(const FVector target, const float separationWeight = 1.0f, const float cohesionweight = 0.5f,
			const float seekWeight = 2.0f, const float avoidanceWeight = 2.0f, const float stepLength = 300, const float acceptanceRadius = 50);
	UFUNCTION(BlueprintCallable, Category = "Flocking")
		bool FlockChaseToLocation(const FVector target, float separationWeight = 0.8f, const float cohesionweight = 0.5f,
			const float seekWeight = 2.0f, const float avoidanceWeight = 2.0f, const float stepLength = 300, const float acceptanceRadius = 50);

	/* --- Command functions --- */
	UFUNCTION(BlueprintCallable, Category = "Command")
		void ExecuteCommand(UCommand* command);
	UFUNCTION(BlueprintCallable, Category = "Command")
		void PopCommand();

	UFUNCTION(BlueprintCallable, Category = "Command")
		void AddCommand_MoveToLocation(const FVector location, const bool isForced, const bool isQueued);
	UFUNCTION(BlueprintCallable, Category = "Command")
		void AddCommand_MoveToEntity(ARTS_Entity* target, const bool isForced, const bool isQueued);
	UFUNCTION(BlueprintCallable, Category = "Command")
		void AddCommand_Stop();
	UFUNCTION(BlueprintCallable, Category = "Command")
		void AddCommand_Attack(ARTS_Entity* target, const bool isForced, const bool isQueued);
	UFUNCTION(BlueprintCallable, Category = "Command")
		void AddCommand_AttackMove(const FVector location, const bool isForced, const bool isQueued);
	UFUNCTION(BlueprintCallable, Category = "Command")
		void AddCommand_CastTarget(AAbility* ability, ARTS_Entity* target, const bool isForced, const bool isQueued);
	UFUNCTION(BlueprintCallable, Category = "Command")
		void AddCommand_CastGround(AAbility* ability, FVector target, const bool isForced, const bool isQueued);
	UFUNCTION(BlueprintImplementableEvent, Category = "Command")
		void UpdateCommandQueueIndicator();
	UFUNCTION(BlueprintCallable, Category = "Command")
		void EnableCommandQueueIndicator(const bool enabled);

public:
	UPROPERTY(BlueprintReadWrite)
		ARTS_Entity * TargetEntity;

protected:
	UPROPERTY(BlueprintReadWrite)
		ARTS_Entity * m_Entity;

	UPROPERTY(BlueprintReadWrite)
		FVector m_TargetLocation;

	UPROPERTY(BlueprintReadOnly)
		EUNIT_TASK m_CurrentTask = EUNIT_TASK::IDLE;
	UPROPERTY(BlueprintReadOnly)
		EUNIT_TASK m_PreviousTask = EUNIT_TASK::IDLE;
	UPROPERTY(BlueprintReadWrite)
		FVector m_LastLocation;
	UPROPERTY(BlueprintReadWrite)
		TArray<UCommand*> m_CommandQueue;
	UPROPERTY(BlueprintReadWrite)
		UCommand* m_CurrentCommand;
	UPROPERTY(BlueprintReadWrite)
		bool m_ShowQueueIndicator = false;
	UPROPERTY(BlueprintReadWrite)
		AAbility* CurrentAbility;

	UPROPERTY(BlueprintReadWrite)
		bool m_IsAlert;

	/* --- Flocking variables --- */
	UPROPERTY(BlueprintReadWrite)
		TArray<ARTS_Entity*> m_NearbyEntities;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float m_FlockingMoveRadius = 200;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float m_FlockingChaseRadius = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float m_ObstructedTimer = 0;

protected:
	UFUNCTION(BlueprintCallable, Category = "Obstruction")
		void UpdateObstructedTimer(const float deltaTime);

private:

	FVector m_FlockCenter;

	FVector FlattenVector(FVector vec);

};