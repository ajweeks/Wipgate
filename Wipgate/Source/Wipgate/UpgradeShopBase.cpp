// Fill out your copyright notice in the Description page of Project Settings.

#include "UpgradeShopBase.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

#include "RTS_PlayerController.h"
#include "RTS_Cursor.h"

DEFINE_LOG_CATEGORY_STATIC(UpgradeShopBaseLog, Log, All);

AUpgradeShopBase::AUpgradeShopBase()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));

	OverlapCube = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("Overlap Cube")));
	OverlapCube->CastShadow = false;
	OverlapCube->SetCanEverAffectNavigation(false);
	OverlapCube->RelativeLocation = FVector(-150.0f, 0.0f, -80.0f);
	OverlapCube->RelativeScale3D = FVector(6.0f, 8.0f, 3.0f);
	OverlapCube->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapCube->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	OverlapCube->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapCube->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	OverlapCube->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}
