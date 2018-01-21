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

	UBoxComponent* BoxComponent;

	// How much to offset the camera from the player spawner at the start of the level
	UPROPERTY(EditAnywhere)
		FVector2D CameraStartingOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere)
		float CameraStartingZoom = 0.0f;

};
