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
}
