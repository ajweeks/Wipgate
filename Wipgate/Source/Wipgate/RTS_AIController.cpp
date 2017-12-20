#include "RTS_AIController.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GeneralFunctionLibrary_CPP.h"

void ARTS_AIController::SetTargetLocation(const FVector target)
{
	m_TargetLocation = target;

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

void ARTS_AIController::RotateTowardsTarget()
{
	FVector start = m_Entity->GetActorLocation();
	FVector end = TargetEntity->GetActorLocation();
	FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(start, end);
	newRotation.Roll = 0;
	newRotation.Pitch = 0;
	m_Entity->SetActorRotation(newRotation);
}

ERelativeAlignment ARTS_AIController::GetRelativeAlignment(const ARTS_Entity * a, const ARTS_Entity * b)
{
	if (a->Team->Alignment == b->Team->Alignment)
		return ERelativeAlignment::E_FRIENDLY;
	else
		return ERelativeAlignment::E_ENEMY;
}

TArray<ARTS_Entity*> ARTS_AIController::GetEnemiesInAttackRange()
{
	TArray<ARTS_Entity*> enemiesInRange;
	FVector loc = m_Entity->GetActorLocation();
	float radius = m_Entity->CurrentAttackStats.Range;
	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	TArray<AActor*> ignore = { m_Entity };
	TArray<AActor*> out;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), loc, radius, objectTypes,
		TSubclassOf<ARTS_Entity>(), ignore, out);
	//DrawDebugSphere(GetWorld(), loc, radius, 6, FColor::White, false, 0, (uint8)'\000', 1);

	ARTS_Entity* temp;
	for (auto entity : out)
	{
		temp = Cast<ARTS_Entity>(entity);
		if (temp != NULL && temp->IsAlive() && temp != m_Entity
			&& GetRelativeAlignment(m_Entity, temp) == ERelativeAlignment::E_ENEMY) {
			enemiesInRange.Add(Cast<ARTS_Entity>(entity));
			//DrawDebugBox(GetWorld(), temp->GetActorLocation(), FVector(20, 20, 100), FColor::White, false, 0, (uint8)'\000', 3);
		}
	}
	return enemiesInRange;
}

bool ARTS_AIController::IsTargetAttacking()
{
	return (GetController(TargetEntity)->GetCurrentTask() == EUNIT_TASK::ATTACKING);
}

void ARTS_AIController::ExecuteCommand(UCommand * command)
{
	switch (command->Type)
	{
	case ECOMMAND_TYPE::NONE:
		break;
	case ECOMMAND_TYPE::STOP:
		break;
	case ECOMMAND_TYPE::MOVE_TO_LOCATION:
		m_CurrentTask = EUNIT_TASK::MOVING;
		SetTargetLocation(Cast<UCommand_MoveToLocation>(command)->Target);
		break;
	case ECOMMAND_TYPE::MOVE_TO_ENTITY:
		m_CurrentTask = EUNIT_TASK::FOLLOWING;
		SetTargetEntity(Cast<UCommand_MoveToEntity>(command)->Target);
		break;
	case ECOMMAND_TYPE::ATTACK_MOVE:
		break;
	case ECOMMAND_TYPE::PATROL:
		break;
	case ECOMMAND_TYPE::ATTACK:
		m_CurrentTask = EUNIT_TASK::ATTACKING;
		SetTargetEntity(Cast<UCommand_Attack>(command)->Target);
		break;
	case ECOMMAND_TYPE::CAST:
		break;
	default:
		break;
	}

	m_CurrentCommand = command;
}

void ARTS_AIController::PopCommand()
{
	if (m_CommandQueue.Num() > 0)
		m_CommandQueue.RemoveAt(0);
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