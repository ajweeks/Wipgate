// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_LevelBounds.h"

#include "Components/BoxComponent.h"

ARTS_LevelBounds::ARTS_LevelBounds()
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
}
