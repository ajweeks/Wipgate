// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Unit.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "AI/Navigation/NavigationPath.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AIController.h"
#include "GeneralFunctionLibrary_CPP.h"
#include "RTS_Squad.h"

DEFINE_LOG_CATEGORY_STATIC(RTS_UNIT_LOG, Log, All);

ARTS_Unit::ARTS_Unit()
{
	USkeletalMeshComponent* mesh = GetMesh();
	if (mesh)
	{
		FRotator rot;
		rot.Yaw = -90;
		mesh->SetRelativeRotation(rot);
	}
}

void ARTS_Unit::SetTeamMaterial()
{
	ARTS_Entity::SetTeamMaterial();

	USkeletalMeshComponent* mesh = GetMesh();
	if (mesh && mesh->GetMaterials().Num() > 0)
	{
		UMaterialInstanceDynamic* bodyMatInst = mesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, mesh->GetMaterial(0));
		bodyMatInst->SetVectorParameterValue("BodyColor", Team.Color);
		mesh->SetReceivesDecals(false);
	}
}

void ARTS_Unit::Kill()
{
	ARTS_Entity::Kill();

	USkeletalMeshComponent* skeletalMesh = GetMesh();
	if (skeletalMesh)
	{
		skeletalMesh->SetCollisionProfileName("Ragdoll");
		skeletalMesh->SetEnableGravity(true);
		skeletalMesh->SetSimulatePhysics(true);
	}
}

void ARTS_Unit::SetCurrentSquad(URTS_Squad * squad)
{
	//Remove from previous squad
	if (CurrentSquad)
	{
		auto index = CurrentSquad->Units.Find(this);
		if(index != INDEX_NONE)
			CurrentSquad->Units.RemoveAt(index);
	}
	CurrentSquad = squad;
}

void ARTS_Unit::SetDirectionLocation(FVector location)
{
	//UNavigationPath *tpath;
	//UNavigationSystem* NavSys = UNavigationSystem::GetCurrent(GetWorld());

	////Get path
	//tpath = NavSys->FindPathToLocationSynchronously(GetWorld(), GetActorLocation(), FinalTarget);
	//UGeneralFunctionLibrary_CPP::DrawPointArray(GetWorld(), tpath->PathPoints, FColor(255, 0, 0), 10.f, 2.f);

	////Check if there are multiple waypoints
	//if (tpath->PathPoints.Num() >= 2)
	//{
	//	//Check distance
	//	if ((tpath->PathPoints[1] - GetActorLocation()).Size() < WaypointMargin &&
	//		tpath->PathPoints.Num() >= 3)
	//	{
	//		CurrentTarget = tpath->PathPoints[2];
	//	}
	//	else
	//	{
	//		CurrentTarget = tpath->PathPoints[1];
	//	}

	//	AAIController* aiControl = Cast<AAIController>(GetController());
	//	if (aiControl)
	//	{
	//		aiControl->MoveToLocation(location, 0.1f, false, true, false, false);
	//	}
	//}
}
