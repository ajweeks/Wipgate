// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RTS_DeathEffect.generated.h"

UCLASS()
class WIPGATE_API ARTS_DeathEffect : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARTS_DeathEffect();

	UPROPERTY(BlueprintReadOnly)
		int Intensity = 0;



	
	
};
