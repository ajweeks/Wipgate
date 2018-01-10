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
#include "Components/StaticMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(RTS_UNIT_LOG, Log, All);

ARTS_Unit::ARTS_Unit()
{
	Headpiece = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Headpiece"));
	Headpiece->SetupAttachment(RootComponent);
	Weapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(RootComponent);

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
			Weapon->SetComponentTickEnabled(false);
			Headpiece->SetComponentTickEnabled(false);

			USkeletalMeshComponent* skeletalMesh = GetMesh();
			//skeletalMesh->SetCollisionProfileName("None");
			skeletalMesh->SetComponentTickEnabled(false);

			//skeletalMesh->SetActive(false);
		}
	}
}

void ARTS_Unit::SetTeamMaterial(URTS_Team* t)
{
	ARTS_Entity::SetTeamMaterial(t);

	//Set skeletal mesh color
	USkeletalMeshComponent* mesh = GetMesh();
	if (mesh && mesh->GetMaterials().Num() > 0)
	{
		UMaterialInstanceDynamic* bodyMatInst = mesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, mesh->GetMaterial(0));
		bodyMatInst->SetVectorParameterValue("TeamColor", t->Color);
		mesh->SetReceivesDecals(false);
	}

	//Set hat team color
	if (Headpiece && Headpiece->GetMaterials().Num() > 0)
	{
		UMaterialInstanceDynamic* hatMatInst = Headpiece->CreateAndSetMaterialInstanceDynamicFromMaterial(0, Headpiece->GetMaterial(0));
		hatMatInst->SetVectorParameterValue("TeamColor", t->Color);
		Headpiece->SetReceivesDecals(false);
	}

	//Disable decals on weapons
	if (Weapon)
	{
		Weapon->SetReceivesDecals(false);
	}
}

void ARTS_Unit::Kill()
{
	UWorld* world = GetWorld();
	if (world)
	{
		//Spawn currency particle
		auto transform = GetActorTransform();
		if (DeathEffectClass)
		{
			world->SpawnActor(DeathEffectClass, &transform);
		}

		ARTS_PlayerController* playercontroller = Cast<ARTS_PlayerController>(world->GetFirstPlayerController());
	if (playercontroller)
	{
		if (Team && (Team->Alignment == ETeamAlignment::E_AGGRESSIVE_AI || Team->Alignment == ETeamAlignment::E_ATTACKEVERYTHING_AI))
		{
			//Add currency
			int32 amount = FMath::RandRange(MinimumCurrencyDrop, MaximumCurrencyDrop);
			playercontroller->AddLuma(amount);
		}
	}
		else
		{
		UE_LOG(RTS_UNIT_LOG, Error, TEXT("Kill > No playercontroller. Returning..."));
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
}

