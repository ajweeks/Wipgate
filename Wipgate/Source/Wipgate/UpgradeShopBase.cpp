// Fill out your copyright notice in the Description page of Project Settings.

#include "UpgradeShopBase.h"

#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

#include "RTS_PlayerController.h"
#include "RTS_Cursor.h"

DEFINE_LOG_CATEGORY_STATIC(UpgradeShopBaseLog, Log, All);

AUpgradeShopBase::AUpgradeShopBase()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(FName(TEXT("Box Collider")));
	BoxCollider->RelativeLocation = FVector(-150.0f, 0.0f, -80.0f);
	BoxCollider->RelativeScale3D = FVector(10.0f, 10.0f, 5.0f);
	BoxCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoxCollider->SetBoxExtent(FVector(100));

	OverlapCube = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("Overlap Cube")));
	OverlapCube->CastShadow = false;
	OverlapCube->SetCanEverAffectNavigation(false);
	OverlapCube->DetachFromParent(true);
	OverlapCube->AttachToComponent(BoxCollider, FAttachmentTransformRules::KeepRelativeTransform);
	OverlapCube->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OverlapCube->RelativeScale3D = FVector(0.65f, 0.65f, 0.65f);
}
