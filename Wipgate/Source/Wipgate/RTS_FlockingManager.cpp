// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_FlockingManager.h"


// Sets default values
ARTS_FlockingManager::ARTS_FlockingManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARTS_FlockingManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARTS_FlockingManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CentreOfMass = GetCentreOfMass();

	for (int32 i = 0; i < Units.Num(); ++i)
	{
		ARTS_Unit* unit = Units[i];
		FVector coherence = (CoherenceBehavior(unit) * unit->CohesionWeight);
		FVector avoid = (AvoidBehavior(unit) * unit->AvoidWeight);
		FVector seek = (SeekBehavior(unit)* unit->SeekWeight);

		FVector output = (unit->GetActorLocation() + ((coherence + avoid + seek) * VectorLengthMultiplier));
		output.Z = GetActorLocation().Z;

		unit->SetDirectionLocation(output);
	}
}

FVector ARTS_FlockingManager::GetCentreOfMass()
{
	FVector output = FVector(0, 0, 0);

	for (int32 i = 0; i < Units.Num(); ++i)
	{
		output += Units[i]->GetActorLocation();
	}
	return output / (Units.Num());
}

FVector ARTS_FlockingManager::CoherenceBehavior(ARTS_Unit * unit)
{
	FVector output = CentreOfMass - unit->GetActorLocation();

	//Adapt weight
	float distancePercent = output.Size() / unit->CohesionDistanceClamp;
	distancePercent = FMath::Clamp(distancePercent, 0.f, 1.f);

	unit->CohesionWeight = unit->CohesionWeightClamp * distancePercent;
	output.Normalize();

	return output;
}

FVector ARTS_FlockingManager::AvoidBehavior(ARTS_Unit * unit)
{
	FVector output = FVector(0.0f, 0.0f, 0.0f);

	for (int32 i = 0; i < Units.Num(); ++i)
	{
		if (Units[i] != unit)
		{
			float d = (Units[i]->GetActorLocation() - unit->GetActorLocation()).Size();
			if (d < UnitAvoidRange)
			{
				output -= (Units[i]->GetActorLocation() - unit->GetActorLocation());
			}
		}
	}

	//Adapt weight
	FVector centre = CentreOfMass - unit->GetActorLocation();

	float distancePercent = output.Size() / unit->AvoidDistanceClamp;
	distancePercent = FMath::Clamp(distancePercent, 0.f, 1.f);

	unit->AvoidWeight = unit->AvoidWeightClamp * (1.f - distancePercent);
	output.Normalize();

	return output;
}

FVector ARTS_FlockingManager::SeekBehavior(ARTS_Unit * unit)
{
	FVector output = FVector(0.0f, 0.0f, 0.0f);;

	output = unit->CurrentTarget - unit->GetActorLocation();
	output.Normalize();

	return output;
}

