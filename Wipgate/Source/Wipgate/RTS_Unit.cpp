// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Unit.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "AI/Navigation/NavigationPath.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "AIController.h"
#include "GeneralFunctionLibrary_CPP.h"
#include "RTS_PlayerController.h"
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
		bodyMatInst->SetVectorParameterValue("TeamColor", Team->Color);
		mesh->SetReceivesDecals(false);
	}
}

void ARTS_Unit::Kill()
{
	//Spawn currency particle
	auto transform = GetActorTransform();
	if (CurrencyEffectClass)
		GetWorld()->SpawnActor(CurrencyEffectClass, &transform);

	ARTS_PlayerController* playercontroller = Cast<ARTS_PlayerController>(GetWorld()->GetFirstPlayerController());
	if (playercontroller && Team->Alignment == ETeamAlignment::E_AGGRESSIVE_AI)
	{
		//Add currency
		int32 amount = FMath::RandRange(MinimumCurrencyDrop, MaximumCurrencyDrop);
		playercontroller->AddLuma(amount);
	}
	else
	{
		UE_LOG(RTS_UNIT_LOG, Error, TEXT("BeginPlay > No playercontroller. Returning..."));
	}

	ARTS_Entity::Kill();

	USkeletalMeshComponent* skeletalMesh = GetMesh();
	if (skeletalMesh)
	{
		skeletalMesh->SetCollisionProfileName("Ragdoll");
		skeletalMesh->SetEnableGravity(true);
		skeletalMesh->SetSimulatePhysics(true);
	}
}

