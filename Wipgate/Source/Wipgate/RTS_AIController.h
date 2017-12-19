#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RTS_Entity.h"
#include "Command.h"

#include "RTS_AIController.generated.h"

UENUM(BlueprintType)
enum class EUNIT_TASK : uint8
{
	IDLE 		UMETA(DisplayName = "Does nothing, reacts to environment"),
	MOVING 		UMETA(DisplayName = "Moving to location"),
	CHASING		UMETA(DisplayName = "Chasing target"),
	FOLLOWING	UMETA(DisplayName = "Following target"),
	ATTACKING	UMETA(DisplayName = "Attacking target"),
	CASTING		UMETA(DisplayName = "Casting ability"),
	EXECUTING	UMETA(DisplayName = "Executing unstoppable task"),
};

UCLASS()
class WIPGATE_API ARTS_AIController : public AAIController
{
	GENERATED_BODY()

	//UFUNCTION(BlueprintCallable, Category = "Setters")
	//	void SetTargetLocation(const FVector target);

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
		float m_FlockingMoveRadius;
	UPROPERTY(BlueprintReadWrite)
		float m_FlockingChaseRadius;
};