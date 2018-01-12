#include "RTS_AIController.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GeneralFunctionLibrary_CPP.h"
#include "RTS_GameState.h"
#include "Runtime/Engine/Classes/AI/Navigation/NavigationSystem.h"
#include "Runtime/Engine/Classes/AI/Navigation/NavigationPath.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"

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
	FVector start = m_FlockCenter;
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
	if (m_Entity->RenderFlockingDebugInfo)
		DrawDebugCircle(GetWorld(), m_FlockCenter - offset, radius, 32, FColor::White, false, 0, (uint8)'\000', 1, FVector(1, 0, 0), FVector(0,1,0), false);
	
	ARTS_GameState * gameState = GetWorld()->GetGameState<ARTS_GameState>();
	TArray<ARTS_Entity*> entities = gameState->Entities;

	for (auto e : entities)
	{
		if (m_Entity != e && FVector::Dist(m_FlockCenter, e->GetActorLocation()) <= radius)
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

		vectorToTarget = e->GetActorLocation() - m_FlockCenter;
		vectorToTarget = FlattenVector(vectorToTarget);
		closenessToTarget = (radius - vectorToTarget.Size()) / radius;

		vectorToTarget.Normalize();
		separation = m_FlockCenter - vectorToTarget * closenessToTarget;
		separationVectors.Add(separation);
	}
	
	// return the normalized average of all separation vectors
	separation = UKismetMathLibrary::GetVectorArrayAverage(separationVectors) - m_FlockCenter;
	separation.Normalize();
	return separation;
}

FVector ARTS_AIController::GetCohesionVector()
{
	TArray<ARTS_Entity*> friendlies = GetFriendlyEntities(m_NearbyEntities);
	friendlies.Add(m_Entity);
	TArray<FVector> positions = GetEntityPositions(friendlies);

	// return normalized vector from entity to center of nearby entities (self inclusive)
	FVector toCenter = UKismetMathLibrary::GetVectorArrayAverage(positions) - m_FlockCenter;
	toCenter = FlattenVector(toCenter);
	toCenter.Normalize();
	return toCenter;
}

FVector ARTS_AIController::GetSeekVector(const FVector target, FVector& nextPathPoint)
{
	TArray<FVector> pathPoints = UNavigationSystem::FindPathToLocationSynchronously(GetWorld(), m_FlockCenter, target)->PathPoints;
	if (pathPoints.Num() >= 2)	
		nextPathPoint = pathPoints[1];
	else 
		nextPathPoint = m_TargetLocation;
	FVector toFirstPathPoint = nextPathPoint - m_FlockCenter;
	toFirstPathPoint = FlattenVector(toFirstPathPoint);
	toFirstPathPoint.Normalize();
	return toFirstPathPoint;
}

FVector ARTS_AIController::GetAvoidanceVector(const FVector nextPathPoint)
{
	FVector avoidance = FVector(0, 0, 0);

	// linetrace
	ECollisionChannel collisionChannel = ECC_Pawn;
	FHitResult hitLeft;
	FHitResult hitRight;
	FVector start = m_Entity->GetActorLocation();
	FVector moveDir = nextPathPoint - start;
	float distanceToTarget = moveDir.Size();
	moveDir.Normalize();
	moveDir = moveDir * FMath::Clamp(300.0f, 0.0f, distanceToTarget - 75); // offset to avoid actual target

	FCollisionQueryParams params;
	params.AddIgnoredActor(m_Entity);

	float radius = m_Entity->GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector right = start + m_Entity->GetActorRightVector() * (radius / 2) ;
	FVector left = start - m_Entity->GetActorRightVector() * (radius / 2);

	bool isRightHit = GetWorld()->LineTraceSingleByChannel(hitRight, right, right + moveDir, collisionChannel, params);
	bool isLeftHit = GetWorld()->LineTraceSingleByChannel(hitLeft, left, left + moveDir, collisionChannel, params);

	if (isRightHit || isLeftHit)
	{
		// calculate average avoidance
		TArray<FVector> obstaclePositions = GetEntityPositions(GetObstacles()); // TODO: take entity size into account

		if (obstaclePositions.Num() <= 0)
			return FVector(0, 0, 0);

		FVector fromObstacle = m_FlockCenter - UKismetMathLibrary::GetVectorArrayAverage(obstaclePositions);
		fromObstacle = FlattenVector(fromObstacle);
		FVector toPathPoint = nextPathPoint - m_FlockCenter;
		toPathPoint = FlattenVector(toPathPoint);

		fromObstacle.Normalize();
		toPathPoint.Normalize();

		FVector avoidanceAverage = toPathPoint + fromObstacle;
		avoidanceAverage.Normalize();

		FHitResult hitOut;
		if (isRightHit)
			hitOut = hitRight;
		if (isLeftHit)
			hitOut = hitLeft;

		// if obstacle is an enemy, make that the target
		ARTS_Entity* hitEntity = Cast<ARTS_Entity>(hitOut.GetActor());
		if (hitEntity)
		{
			if (GetRelativeAlignment(hitEntity, m_Entity) == ERelativeAlignment::E_ENEMY)
				SetTargetEntity(hitEntity);
		}

		//if (m_CurrentTask == EUNIT_TASK::CHASING)
		//{
		//	// is obstacle left or right?
		//	FVector hitLoc = hitOut.GetActor()->GetActorLocation();
		//	FVector toObstacle = hitOut.GetActor()->GetActorLocation() - start;
		//	toObstacle.Normalize();
		//	FVector toHit = hitLoc - start;
		//	toHit.Normalize();
		//	//DrawDebugLine(GetWorld(), start, hitOut.GetActor()->GetActorLocation(), FColor::Black, false, 0, (uint8)'\000', 2);

		//	if (isRightHit)
		//	{
		//		avoidance = UKismetMathLibrary::Cross_VectorVector(toObstacle, m_Entity->GetActorUpVector());
		//		avoidance.Normalize();
		//		avoidance *= 1;
		//	}
		//	else
		//	{
		//		avoidance = UKismetMathLibrary::Cross_VectorVector(toObstacle, m_Entity->GetActorUpVector());
		//		avoidance.Normalize();
		//		avoidance *= -1;
		//	}
		//	avoidance.Normalize();
		//}
		avoidance += avoidanceAverage;
		avoidance.Normalize();
	}

	return avoidance;
}

TArray<ARTS_Entity*> ARTS_AIController::GetObstacles()
{
	TArray<ARTS_Entity*> obstacles;
	if (m_CurrentTask == EUNIT_TASK::CHASING)
	{
		// forced attack entity: everyone is an obstacle
		if (m_CurrentCommand && m_CurrentCommand->Type == ECOMMAND_TYPE::ATTACK)
		{
			for (auto e : m_NearbyEntities)
				if (e != TargetEntity)
					obstacles.Add(e);
		}
		// attack move or chasing: allies are an obstacle
		else
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

			if (!controller) continue;
			if (GetRelativeAlignment(m_Entity, e) == ERelativeAlignment::E_ENEMY) {
				obstacles.Add(e);
				continue;
			}
			
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
	FVector offset = FVector(0, 0, m_Entity->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 15);
	FVector start = m_FlockCenter - offset;
	float step = stepLength * 2;

	// separation
	FVector end = start + separation * step - offset;
	DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 0, (uint8)'\000', 3);
	// cohesion
	end = start + cohesion * step - offset;
	DrawDebugLine(GetWorld(), start, end, FColor::Blue, false, 0, (uint8)'\000', 3);
	// seek
	end = start + seek * step - offset;
	DrawDebugLine(GetWorld(), start, end, FColor::Green, false, 0, (uint8)'\000', 3);
	// avoidance
	end = start + avoidance * step - offset;
	DrawDebugLine(GetWorld(), start, end, FColor::Purple, false, 0, (uint8)'\000', 3);

	//UE_LOG(LogTemp, Log, TEXT("cohesion length %f"), cohesion.Size());
	//UE_LOG(LogTemp, Log, TEXT("sep length %f"), separation.Size());
	//UE_LOG(LogTemp, Log, TEXT("seek length %f"), seek.Size());
	//UE_LOG(LogTemp, Log, TEXT("avoidance length %f"), avoidance.Size());
}

bool ARTS_AIController::FlockMoveToLocation(const FVector target, const float separationWeight, const float cohesionweight, const float seekWeight, const float avoidanceWeight, const float stepLength, const float acceptanceRadius)
{
	//m_FlockTick++;
	//if (m_FlockTick % 3 != 0) return false;

	m_FlockCenter = m_Entity->GetActorLocation();
	StoreNearbyEntities(m_FlockingMoveRadius);

	if (m_NearbyEntities.Num() == 0)
		MoveToLocation(target);
	else
	{
		FVector separation = GetSeparationVector(m_FlockingMoveRadius) * separationWeight;
		FVector cohesion = GetCohesionVector() * cohesionweight;
		FVector nextPathPoint;
		FVector seek = GetSeekVector(target, nextPathPoint) * seekWeight;
		FVector avoidance = GetAvoidanceVector(nextPathPoint) * avoidanceWeight;


		if (m_Entity->RenderFlockingDebugInfo)
			RenderFlockingDebug(separation, cohesion, seek, avoidance, stepLength);
		
		FVector newTarget = separation + cohesion + seek + avoidance;
		newTarget.Normalize();
		newTarget *= stepLength;
		newTarget += m_FlockCenter;

		MoveToLocation(newTarget);
	}

	FVector pos2D = FlattenVector(m_FlockCenter);
	FVector target2D = FlattenVector(target);
	float distanceToTarget = (pos2D - target2D).Size();

	if (distanceToTarget < acceptanceRadius) 
		return true;

	return false;
}

bool ARTS_AIController::FlockChaseToLocation(const FVector target, float separationWeight, const float cohesionweight, const float seekWeight, const float avoidanceWeight, const float stepLength, const float acceptanceRadius)
{
	//m_FlockTick++;
	//if (m_FlockTick % 2 != 0) return false;

	m_FlockCenter = m_Entity->GetActorLocation();
	StoreNearbyEntities(m_FlockingChaseRadius);

	if (m_NearbyEntities.Num() == 0)
		MoveToLocation(target);
	else
	{
		FVector nextPathPoint;
		FVector seek = GetSeekVector(target, nextPathPoint) * seekWeight;
		FVector avoidance = GetAvoidanceVector(nextPathPoint) * avoidanceWeight;

		separationWeight *= 1 + m_ObstructedTimer * 30;
		FVector separation = GetSeparationVector(m_FlockingChaseRadius) * separationWeight;
		FVector cohesion = GetCohesionVector() * cohesionweight;
		//UE_LOG(LogTemp, Log, TEXT("Avoidance length: %f, Avoidance weight: %f"), avoidance.Size(), avoidanceWeight);

		if (m_Entity->RenderFlockingDebugInfo)
			RenderFlockingDebug(separation, cohesion, seek, avoidance, stepLength);

		// increase avoidance when slowed down
		float maxSpeed = m_Entity->CurrentMovementStats.Speed;
		float speedFactor = 5;
		float avoidanceMultiplier = speedFactor * (maxSpeed - m_Entity->GetVelocity().Size()) / (maxSpeed / 1.5f);

		FVector newTarget = separation + cohesion + seek + avoidance * (1 + 0.5 * avoidanceMultiplier);
		newTarget.Normalize();
		newTarget *= stepLength;
		newTarget += m_FlockCenter;

		MoveToLocation(newTarget);
	}

	FVector pos2D = FlattenVector(m_FlockCenter);
	FVector target2D = FlattenVector(target);
	float distanceToTarget = (pos2D - target2D).Size();

	if (distanceToTarget < acceptanceRadius)
		return true;

	return false;
}

ERelativeAlignment ARTS_AIController::GetRelativeAlignment(const ARTS_Entity * a, const ARTS_Entity * b)
{
	if (a->Alignment == ETeamAlignment::E_ATTACKEVERYTHING_AI || b->Alignment == ETeamAlignment::E_ATTACKEVERYTHING_AI) 
		return ERelativeAlignment::E_ENEMY;
	if (a->Team->Alignment == b->Team->Alignment || a->Alignment == ETeamAlignment::E_NEUTRAL_AI || b->Alignment == ETeamAlignment::E_NEUTRAL_AI) 
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
	float radius = m_Entity->CurrentVisionStats.OuterRange;
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

bool ARTS_AIController::IsTargetAgressive()
{
	EUNIT_TASK targetTask = GetController(TargetEntity)->GetCurrentTask();
	return (targetTask  == EUNIT_TASK::ATTACKING
		|| targetTask == EUNIT_TASK::CHASING);
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
	if (!IsValid(command) && m_CurrentTask != EUNIT_TASK::CASTING 
		&& m_CurrentTask != EUNIT_TASK::EXECUTING)
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
	case ECOMMAND_TYPE::CAST_ON_TARGET:
		m_CurrentTask = EUNIT_TASK::CHASING;
		SetTargetEntity(Cast<UCommand_CastTarget>(command)->Target);
		break;
	case ECOMMAND_TYPE::CAST_ON_GROUND:
		m_CurrentTask = EUNIT_TASK::CHASING;
		SetTargetLocation(Cast<UCommand_CastGround>(command)->Target);
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
	m_CurrentCommand = nullptr;
}

void ARTS_AIController::AddCommand_MoveToLocation(const FVector location, const bool isForced, const bool isQueued)
{
	UCommand_MoveToLocation* command = NewObject<UCommand_MoveToLocation>(this);
	command->Target = location;
	command->IsForced = isForced;

	if (!isQueued && m_CurrentTask != EUNIT_TASK::EXECUTING && m_CurrentTask != EUNIT_TASK::CASTING)
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

void ARTS_AIController::AddCommand_CastTarget(AAbility * ability, ARTS_Entity * target, const bool isForced, const bool isQueued)
{
	UCommand_CastTarget* command = NewObject<UCommand_CastTarget>(this);
	command->Ability = ability;
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

void ARTS_AIController::UpdateObstructedTimer(const float deltaTime)
{
	if (m_CurrentTask == EUNIT_TASK::CHASING && m_Entity->GetVelocity().Size() <= 60)
		m_ObstructedTimer += deltaTime;
	else if (m_CurrentTask == EUNIT_TASK::CHASING && m_Entity->GetVelocity().Size() >= 60)
		m_ObstructedTimer = 0;
	return;
}

FVector ARTS_AIController::FlattenVector(FVector vec)
{
	vec.Z = 0;
	return vec;
}