#include "RTS_AIController.h"
#include "GeneralFunctionLibrary_CPP.h"

//void ARTS_AIController::SetTargetLocation(const FVector target)
//{
//	m_TargetLocation = target;
//	m_CurrentTask = EUNIT_TASK::MOVING;
//
//	if (GetController(TargetEntity))
//		GetController(TargetEntity)->m_IsAlert = false;
//}

void ARTS_AIController::AddCommand_MoveToLocation(const FVector location, const bool isForced, const bool isQueued)
{
	UCommand_MoveToLocation* command = NewObject<UCommand_MoveToLocation>(this);
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