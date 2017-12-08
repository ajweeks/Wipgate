// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Unit.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"

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
	}
}

void ARTS_Unit::Kill()
{

}
