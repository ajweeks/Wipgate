// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Structure.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"

ARTS_Structure::ARTS_Structure()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMeshComponent->SetupAttachment(RootComponent);
}

void ARTS_Structure::SetTeamMaterial()
{
	ARTS_Entity::SetTeamMaterial();

	if (StaticMeshComponent && StaticMeshComponent->GetMaterials().Num() > 0)
	{
		UMaterialInstanceDynamic* matInst = StaticMeshComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, StaticMeshComponent->GetMaterial(0));
		matInst->SetVectorParameterValue("BodyColor", Team.Color);
	}
}