// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RTS_LevelBounds.generated.h"

class UBoxComponent;

UCLASS()
class WIPGATE_API ARTS_LevelBounds : public AActor
{
	GENERATED_BODY()
	
public:	
	ARTS_LevelBounds();

	UBoxComponent * BoxComponent;

};
