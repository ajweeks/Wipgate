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

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	OverlapCube = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("Overlap Cube")));
	OverlapCube->CastShadow = false;
	OverlapCube->SetCanEverAffectNavigation(false);
	OverlapCube->RelativeLocation = FVector(180.0f, 0.0f, -80.0f);
	OverlapCube->RelativeScale3D = FVector(6.0f, 8.0f, 3.0f);
}

void AUpgradeShopBase::BeginPlay()
{
	m_PlayerControllerRef = Cast<ARTS_PlayerController>(GetWorld()->GetFirstPlayerController());

	OverlapCube->OnBeginCursorOver.AddDynamic(this, &AUpgradeShopBase::OnOverlapCubeBeginCursorOver);
	OverlapCube->OnEndCursorOver.AddDynamic(this, &AUpgradeShopBase::OnOverlapCubeEndCursorOver);
}

void AUpgradeShopBase::OnOverlapCubeBeginCursorOver(UPrimitiveComponent* Component)
{
	UE_LOG(UpgradeShopBaseLog, Error, TEXT("begin!"));
	m_PlayerControllerRef->CursorRef->SetCursorTexture(m_PlayerControllerRef->CursorRef->ShopTexture);
}

void AUpgradeShopBase::OnOverlapCubeEndCursorOver(UPrimitiveComponent* Component)
{
	UE_LOG(UpgradeShopBaseLog, Error, TEXT("end!"));
	if (m_PlayerControllerRef->CursorRef->CurrentTexture == m_PlayerControllerRef->CursorRef->ShopTexture)
	{
		m_PlayerControllerRef->CursorRef->SetCursorTexture(m_PlayerControllerRef->CursorRef->DefaultTexture);
	}
}
