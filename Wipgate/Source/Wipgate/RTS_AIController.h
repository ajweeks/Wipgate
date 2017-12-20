#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RTS_Entity.h"
#include "Command.h"

#include "RTS_AIController.generated.h"

UENUM(BlueprintType)
enum class EUNIT_TASK : uint8
{
	IDLE 		UMETA(DisplayName = "Idle"),
	MOVING 		UMETA(DisplayName = "Moving"),
	CHASING		UMETA(DisplayName = "Chasing"),
	FOLLOWING	UMETA(DisplayName = "Following"),
	ATTACKING	UMETA(DisplayName = "Attacking"),
	CASTING		UMETA(DisplayName = "Casting "),
	EXECUTING	UMETA(DisplayName = "Executing"),
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

	UFUNCTION(BlueprintPure, Category = "Getters")
		UCommand* GetCurrentCommand() { return m_CurrentCommand; }
	UFUNCTION(BlueprintPure, Category = "Getters")
		EUNIT_TASK GetCurrentTask() { return m_CurrentTask; }
	UFUNCTION(BlueprintPure, Category = "Getters")
		bool IsAlert() { return m_IsAlert; }

	/* --- AI functions --- */
	UFUNCTION(BlueprintCallable, Category = "AI")
		TArray<ARTS_Entity*> GetEntitiesWithTask(const TArray<ARTS_Entity*> entities, const EUNIT_TASK task);
	UFUNCTION(BlueprintCallable, Category = "AI")
		ARTS_Entity* GetClosestEntity(const TArray<ARTS_Entity*> entities);
	UFUNCTION(BlueprintCallable, Category = "AI")
		void RotateTowardsTarget();

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

public:
	UPROPERTY(BlueprintReadWrite)
		ARTS_Entity * TargetEntity;

protected:
	UPROPERTY(BlueprintReadWrite)
		ARTS_Entity * m_Entity;

	UPROPERTY(BlueprintReadWrite)
		FVector m_TargetLocation;

	UPROPERTY(BlueprintReadWrite)
		EUNIT_TASK m_CurrentTask = EUNIT_TASK::IDLE;
	UPROPERTY(BlueprintReadWrite)
		FVector m_LastLocation;
	UPROPERTY(BlueprintReadWrite)
		TArray<UCommand*> m_CommandQueue;
	UPROPERTY(BlueprintReadWrite)
		UCommand* m_CurrentCommand;

	UPROPERTY(BlueprintReadWrite)
		bool m_IsAlert;

	/* --- Flocking variables --- */
	UPROPERTY(BlueprintReadWrite)
		TArray<ARTS_Entity*> m_NearbyEntities;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float m_FlockingMoveRadius = 200;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float m_FlockingChaseRadius = 100;

private:
	ARTS_AIController* GetController(ARTS_Entity* entity);
};