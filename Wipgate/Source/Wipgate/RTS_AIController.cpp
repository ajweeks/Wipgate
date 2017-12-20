#include "RTS_AIController.h"
#include "GeneralFunctionLibrary_CPP.h"

void ARTS_AIController::SetTargetLocation(const FVector target)
{
	m_TargetLocation = target;
	m_CurrentTask = EUNIT_TASK::MOVING;

 	if (TargetEntity)
	{
		if (GetController(TargetEntity))
			GetController(TargetEntity)->m_IsAlert = false;
	}
}

void ARTS_AIController::SetTargetEntity(ARTS_Entity * targetEntity)
{
	if (TargetEntity)
	{
		if (GetController(TargetEntity))
			GetController(TargetEntity)->m_IsAlert = false;
	}

	TargetEntity = targetEntity;
}

TArray<ARTS_Entity*> ARTS_AIController::GetEntitiesWithTask(const TArray<ARTS_Entity*> entities, const EUNIT_TASK task)
{
	TArray<ARTS_Entity*> entitiesWithTask;
	for (int i = 0; i < entities.Num(); i++)
	{
		if (GetController(entities[i])->GetCurrentTask() == task)
			entitiesWithTask.Add(entities[i]);
	}
	return entitiesWithTask;
}

ARTS_Entity* ARTS_AIController::GetClosestEntity(const TArray<ARTS_Entity*> entities)
{
	ARTS_Entity* closestEntity = nullptr;
	float shortestDistance = TNumericLimits<float>::Max();
	for (int i = 0; i < entities.Num(); i++)
	{
		if (m_Entity->GetDistanceTo(entities[i]) < shortestDistance)
		{
			closestEntity = entities[i];
			shortestDistance = m_Entity->GetDistanceTo(closestEntity);
		}
	}
	return closestEntity;
}

void ARTS_AIController::AddCommand_MoveToLocation(const FVector location, const bool isForced, const bool isQueued)
{
	UCommand_MoveToLocation* command = NewObject<UCommand_MoveToLocation>(this);
	command->Target = location;
	command->IsForced = isForced;

	if (!isQueued && m_CurrentTask != EUNIT_TASK::EXECUTING)
		m_CommandQueue.Empty();

	m_CommandQueue.Add(command);
}

void ARTS_AIController::AddCommand_MoveToEntity(ARTS_Entity * target, const bool isForced, const bool isQueued)
{
	UCommand_MoveToEntity* command = NewObject<UCommand_MoveToEntity>(this);
	command->Target = target;
	command->IsForced = isForced;

	if (!isQueued && m_CurrentTask != EUNIT_TASK::EXECUTING)
		m_CommandQueue.Empty();

	m_CommandQueue.Add(command);
}

void ARTS_AIController::AddCommand_Stop()
{
	m_CommandQueue.Empty();
	StopMovement();
	m_CurrentTask = EUNIT_TASK::IDLE;
}

void ARTS_AIController::AddCommand_Attack(ARTS_Entity * target, const bool isForced, const bool isQueued)
{
	UCommand_Attack* command = NewObject<UCommand_Attack>(this);
	command->Target = target;
	command->IsForced = isForced;

	if (!isQueued && m_CurrentTask != EUNIT_TASK::EXECUTING)
		m_CommandQueue.Empty();

	m_CommandQueue.Add(command);
}

ARTS_AIController* ARTS_AIController::GetController(ARTS_Entity* entity)
{
	AAIController* controller = Cast<AAIController>(entity->GetController());
	return Cast<ARTS_AIController>(controller);
}