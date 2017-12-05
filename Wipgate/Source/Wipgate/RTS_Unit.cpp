// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Unit.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

DEFINE_LOG_CATEGORY(RTS_UNIT_LOG);

ARTS_Unit::ARTS_Unit()
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	FRotator rot;
	rot.Yaw = -90;
	SkeletalMeshComponent->SetRelativeRotation(rot);
}

void ARTS_Unit::SetTeamMaterial()
{
	ARTS_Entity::SetTeamMaterial();

	if (SkeletalMeshComponent && SkeletalMeshComponent->GetMaterials().Num() > 0)
	{
		UMaterialInstanceDynamic* bodyMatInst = SkeletalMeshComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, SkeletalMeshComponent->GetMaterial(0));
		bodyMatInst->SetVectorParameterValue("BodyColor", Team.Color);
	}
}
