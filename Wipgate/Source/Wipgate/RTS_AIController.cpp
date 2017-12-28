#include "RTS_AIController.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GeneralFunctionLibrary_CPP.h"
#include "RTS_GameState.h"
#include "Runtime/Engine/Classes/AI/Navigation/NavigationSystem.h"
#include "Runtime/Engine/Classes/AI/Navigation/NavigationPath.h"

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

void ARTS_AIController::SetCurrentTask(const EUNIT_TASK task)
{
	m_PreviousTask = m_CurrentTask;
	m_CurrentTask = task;
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

void ARTS_AIController::StoreNearbyEntities(const float radius)
{
	m_NearbyEntities.Empty();
	if (!m_Entity->IsAlive()) return; 

	FVector offset (0, 0, m_Entity->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 5);
	DrawDebugCircle(GetWorld(), m_Entity->GetActorLocation() - offset, radius, 32, FColor::White, false, 0, (uint8)'\000', 1, FVector(1, 0, 0), FVector(0,1,0), false);
	ARTS_GameState * gameState = GetWorld()->GetGameState<ARTS_GameState>();
	TArray<ARTS_Entity*> entities = gameState->Entities;

	for (auto e : entities)
	{
		if (m_Entity != e && m_Entity->GetDistanceTo(e) <= radius)
		{
			if (e->IsAlive())
				m_NearbyEntities.Add(e);
		}
	}
}

FVector ARTS_AIController::GetSeparationVector(const float radius)
{
	if (m_NearbyEntities.Num() == 0) return FVector(0, 0, 0);

	FVector vectorToTarget;
	float closenessToTarget; // 0 to 1
	TArray<FVector> separationVectors;
	FVector separation;

	// get separation for each nearby entity
	for (auto e : m_NearbyEntities)
	{
		// don't separate from target
		if (e == TargetEntity) continue;

		vectorToTarget = e->GetActorLocation() - m_Entity->GetActorLocation();
		closenessToTarget = (radius - vectorToTarget.Size()) / radius;

		vectorToTarget.Normalize();
		separation = m_Entity->GetActorLocation() - vectorToTarget * closenessToTarget;
		separationVectors.Add(separation);
	}
	
	// return the normalized average of all separation vectors
	separation = UKismetMathLibrary::GetVectorArrayAverage(separationVectors) - m_Entity->GetActorLocation();
	separation.Normalize();
	return separation;
}

FVector ARTS_AIController::GetCohesionVector()
{
	TArray<ARTS_Entity*> friendlies = GetFriendlyEntities(m_NearbyEntities);
	friendlies.Add(m_Entity);
	TArray<FVector> positions = GetEntityPositions(friendlies);

	// return normalized vector from entity to center of nearby entities (self inclusive)
	FVector toCenter = UKismetMathLibrary::GetVectorArrayAverage(positions) - m_Entity->GetActorLocation();
	toCenter.Normalize();
	return toCenter;
}

FVector ARTS_AIController::GetSeekVector(const FVector target, FVector& nextPathPoint)
{
	TArray<FVector> pathPoints = UNavigationSystem::FindPathToLocationSynchronously(GetWorld(), m_Entity->GetActorLocation(), target)->PathPoints;
	if (pathPoints.Num() >= 2)	
		nextPathPoint = pathPoints[1];
	else 
		nextPathPoint = m_TargetLocation;
	FVector toFirstPathPoint = nextPathPoint - m_Entity->GetActorLocation();
	toFirstPathPoint.Normalize();
	return toFirstPathPoint;
}

FVector ARTS_AIController::GetAvoidanceVector(const FVector nextPathPoint)
{
	TArray<FVector> obstaclePositions = GetEntityPositions(GetObstacles()); // TODO: take entity size into account

	if (obstaclePositions.Num() <= 0)
		return FVector(0, 0, 0);

	FVector fromObstacle = m_Entity->GetActorLocation() - UKismetMathLibrary::GetVectorArrayAverage(obstaclePositions);
	FVector toPathPoint = nextPathPoint - m_Entity->GetActorLocation();
	// extra avoid priority based on distance to obstacle and next path point
	float avoidPriority = fromObstacle.Size() / toPathPoint.Size();

	fromObstacle.Normalize();
	toPathPoint.Normalize();

	FVector avoidance = toPathPoint + fromObstacle * (1 + avoidPriority);
	avoidance.Normalize();
	return avoidance;
}

TArray<ARTS_Entity*> ARTS_AIController::GetObstacles()
{
	TArray<ARTS_Entity*> obstacles;
	if (m_CurrentTask == EUNIT_TASK::CHASING)
	{
		// forced attack entity: everyone is an obstacle
		if (m_CurrentCommand->Type == ECOMMAND_TYPE::ATTACK)
		{
			for (auto e : m_NearbyEntities)
				if (e != TargetEntity)
					obstacles.Add(e);
		}
		// attack move: allies are an obstacle
		else if (m_CurrentCommand->Type == ECOMMAND_TYPE::ATTACK_MOVE)
		{
			for (auto e : m_NearbyEntities)
				if (GetRelativeAlignment(m_Entity, e) == ERelativeAlignment::E_FRIENDLY)
					obstacles.Add(e);
		}
	}

	// move: enemies and allies that stand still are an obstacle
	if (m_CurrentTask == EUNIT_TASK::MOVING
		|| m_CurrentTask == EUNIT_TASK::FOLLOWING
		|| m_CurrentTask == EUNIT_TASK::ATTACK_MOVING)
	{
		ARTS_AIController* controller;
		for (auto e : m_NearbyEntities)
		{
			controller = GetController(e);

			if (GetRelativeAlignment(m_Entity, e) == ERelativeAlignment::E_ENEMY)
				obstacles.Add(e);

			if (!controller) continue;
			if (controller->GetCurrentTask() == EUNIT_TASK::ATTACKING
				|| controller->GetCurrentTask() == EUNIT_TASK::IDLE
				|| controller->GetCurrentTask() == EUNIT_TASK::CASTING)
				obstacles.Add(e);
		}

	}
	return obstacles;
}

void ARTS_AIController::RenderFlockingDebug(const FVector separation, const FVector cohesion, const FVector seek, const FVector avoidance, float stepLength)
{
	FVector start = m_Entity->GetActorLocation() - FVector(0, 0, m_Entity->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 15);
	// separation
	FVector end = start + separation * stepLength;
	DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 0, (uint8)'\000', 3);
	// cohesion
	end = start + cohesion * stepLength;
	DrawDebugLine(GetWorld(), start, end, FColor::Blue, false, 0, (uint8)'\000', 3);
	// seek
	end = start + seek * stepLength;
	DrawDebugLine(GetWorld(), start, end, FColor::Green, false, 0, (uint8)'\000', 3);
	// avoidance
	end = start + avoidance * stepLength;
	DrawDebugLine(GetWorld(), start, end, FColor::Purple, false, 0, (uint8)'\000', 3);
}

bool ARTS_AIController::FlockMoveToLocation(const FVector target, const float separationWeight, const float cohesionweight, const float seekWeight, const float avoidanceWeight, const float stepLength, const float acceptanceRadius)
{
	StoreNearbyEntities(m_FlockingMoveRadius);

	if (m_NearbyEntities.Num() == 0)
		MoveToLocation(target);
	else
	{
		FVector separation = GetSeparationVector(m_FlockingMoveRadius) * separationWeight;
		FVector cohesion = GetCohesionVector() * cohesionweight;
		FVector nextPathPoint;
		FVector seek = GetSeekVector(target, nextPathPoint) * seekWeight;
		FVector avoidance = GetAvoidanceVector(nextPathPoint);

		RenderFlockingDebug(separation, cohesion, seek, avoidance, stepLength);
		FVector newTarget = separation + cohesion + seek + avoidance;
		newTarget.Normalize();
		newTarget *= stepLength;
		newTarget += m_Entity->GetActorLocation();

		MoveToLocation(newTarget);
	}

	FVector pos2D = FlattenVector(m_Entity->GetActorLocation());
	FVector target2D = FlattenVector(target);
	float distanceToTarget = (pos2D - target2D).Size();

	// standard acceptance radius
	if (distanceToTarget < acceptanceRadius) 
		return true;
	// acceptance radius relative to velocity and nearby entity count
	else if (distanceToTarget <= m_NearbyEntities.Num() * 30
		&& m_Entity->GetVelocity().Size() <= 30)
		return true;

	return false;
}

bool ARTS_AIController::FlockChaseToLocation(const FVector target, const float separationWeight, const float cohesionweight, const float seekWeight, const float avoidanceWeight, const float stepLength, const float acceptanceRadius)
{
	StoreNearbyEntities(m_FlockingChaseRadius);

	if (m_NearbyEntities.Num() == 0)
		MoveToLocation(target);
	else
	{
		FVector separation = GetSeparationVector(m_FlockingChaseRadius) * separationWeight;
		FVector cohesion = GetCohesionVector() * cohesionweight;
		FVector nextPathPoint;
		FVector seek = GetSeekVector(target, nextPathPoint) * seekWeight;
		FVector avoidance = GetAvoidanceVector(nextPathPoint);

		RenderFlockingDebug(separation, cohesion, seek, avoidance, stepLength);

		// increase avoidance when slowed down
		float maxSpeed = m_Entity->CurrentMovementStats.Speed;
		float speedFactor = 5;
		float avoidanceMultiplier = 5 * (maxSpeed - m_Entity->GetVelocity().Size()) / maxSpeed;

		FVector newTarget = separation + cohesion + seek + avoidance * (1 + avoidanceMultiplier);
		newTarget.Normalize();
		newTarget *= stepLength;
		newTarget += m_Entity->GetActorLocation();

		MoveToLocation(newTarget);
	}

	FVector pos2D = FlattenVector(m_Entity->GetActorLocation());
	FVector target2D = FlattenVector(target);
	float distanceToTarget = (pos2D - target2D).Size();

	if (distanceToTarget < acceptanceRadius)
		return true;

	return false;
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

TArray<ARTS_Entity*> ARTS_AIController::GetEnemiesInVisionRange()
{
	TArray<ARTS_Entity*> enemiesInRange;
	FVector loc = m_Entity->GetActorLocation();
	float radius = m_Entity->CurrentVisionStats.InnerRange;
	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	TArray<AActor*> ignore = { m_Entity };
	TArray<AActor*> out;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), loc, radius, objectTypes,
		TSubclassOf<ARTS_Entity>(), ignore, out);

	ARTS_Entity* temp;
	for (auto entity : out)
	{
		temp = Cast<ARTS_Entity>(entity);
		if (temp != NULL && temp->IsAlive() && temp != m_Entity
			&& GetRelativeAlignment(m_Entity, temp) == ERelativeAlignment::E_ENEMY) {
			enemiesInRange.Add(Cast<ARTS_Entity>(entity));
		}
	}
	return enemiesInRange;
}

bool ARTS_AIController::IsTargetAttacking()
{
	return (GetController(TargetEntity)->GetCurrentTask() == EUNIT_TASK::ATTACKING);
}

TArray<ARTS_Entity*> ARTS_AIController::GetFriendlyEntities(const TArray<ARTS_Entity*> entities)
{
	TArray<ARTS_Entity*> friendlies;
	for (auto e : entities)
	{
		if (GetRelativeAlignment(m_Entity, e) == ERelativeAlignment::E_FRIENDLY)
			friendlies.Add(e);
	}
	return friendlies;
}

TArray<FVector> ARTS_AIController::GetEntityPositions(const TArray<ARTS_Entity*> entities)
{
	TArray<FVector> positions;
	for (auto e : entities)
	{
		positions.Add(e->GetActorLocation());
	}
	return positions;
}

void ARTS_AIController::ExecuteCommand(UCommand * command)
{
	if (!IsValid(command))
	{
		m_CurrentTask = EUNIT_TASK::IDLE;
		return;
	}

	switch (command->Type)
	{
	case ECOMMAND_TYPE::NONE:
		break;
	case ECOMMAND_TYPE::STOP:
		break;
	case ECOMMAND_TYPE::MOVE_TO_LOCATION:
		SetCurrentTask(EUNIT_TASK::MOVING);
		SetTargetLocation(Cast<UCommand_MoveToLocation>(command)->Target);
		break;
	case ECOMMAND_TYPE::MOVE_TO_ENTITY:
		m_CurrentTask = EUNIT_TASK::FOLLOWING;
		SetTargetEntity(Cast<UCommand_MoveToEntity>(command)->Target);
		break;
	case ECOMMAND_TYPE::ATTACK_MOVE:
		m_CurrentTask = EUNIT_TASK::ATTACK_MOVING;
		SetTargetLocation(Cast<UCommand_AttackMove>(command)->Target);
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
	SetCurrentTask(EUNIT_TASK::IDLE);
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

void ARTS_AIController::AddCommand_AttackMove(const FVector location, const bool isForced, const bool isQueued)
{
	UCommand_AttackMove* command = NewObject<UCommand_AttackMove>(this);
	command->Target = location;
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

FVector ARTS_AIController::FlattenVector(FVector vec)
{
	vec.Z = 0;
	return vec;
}