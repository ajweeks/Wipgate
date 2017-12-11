// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RTS_Squad.generated.h"

class ARTS_Unit;

/**
 * 
 */
UCLASS()
class WIPGATE_API URTS_Squad : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	TArray<ARTS_Unit*> Units;

	//Functions
	void Update(float dt);
	
	//Steering
	FVector GetCentreOfMass();
	FVector CoherenceBehavior(ARTS_Unit* unit);
	FVector AvoidBehavior(ARTS_Unit* unit);
	FVector SeekBehavior(ARTS_Unit* unit);

	//Properties
	//TODO: Editable
	float UnitAvoidRange = 75.f;
	float VectorLengthMultiplier = 5.f;

	//Variables
	FVector CentreOfMass;
	float CurrentTimer = 0.f;
	const float MaxTimer = 0.1f;
	UPROPERTY(BlueprintReadWrite)
		FVector ClickedTarget;
};
