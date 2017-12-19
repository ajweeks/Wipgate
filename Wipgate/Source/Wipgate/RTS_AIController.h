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
	//UFUNCTION(BlueprintCallable, Category = "Setters")
	//	void SetTargetLocation(const FVector target);
	UFUNCTION(BlueprintCallable, Category = "Getters")
		UCommand* GetCurrentCommand() { return m_CurrentCommand; }
	UFUNCTION(BlueprintCallable, Category = "Getters")
		bool IsAlert() { return m_IsAlert; }

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
};