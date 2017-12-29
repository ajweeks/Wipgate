// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Unit.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "AI/Navigation/NavigationPath.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AIController.h"
#include "GeneralFunctionLibrary_CPP.h"
#include "RTS_Team.h"

DEFINE_LOG_CATEGORY_STATIC(RTS_UNIT_LOG, Log, All);

ARTS_Unit::ARTS_Unit()
{
	USkeletalMeshComponent* mesh = GetMesh();
	if (mesh)
	{
		FRotator rot = FRotator(0, -90, 0);
		mesh->SetRelativeRotation(rot);
	}
}

void ARTS_Unit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_PostKillTimer < POSTKILLDELAY && Health <= 0)
	{
		m_PostKillTimer += DeltaTime;
		if (m_PostKillTimer > POSTKILLDELAY)
		{
			USkeletalMeshComponent* skeletalMesh = GetMesh();
			//skeletalMesh->SetCollisionProfileName("None");
			skeletalMesh->SetComponentTickEnabled(false);

			//skeletalMesh->SetActive(false);
		}
	}
}

void ARTS_Unit::SetTeamMaterial()
{
	ARTS_Entity::SetTeamMaterial();

	USkeletalMeshComponent* mesh = GetMesh();
	if (mesh && mesh->GetMaterials().Num() > 0)
	{
		UMaterialInstanceDynamic* bodyMatInst = mesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, mesh->GetMaterial(0));
		bodyMatInst->SetVectorParameterValue("BodyColor", Team->Color);
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

