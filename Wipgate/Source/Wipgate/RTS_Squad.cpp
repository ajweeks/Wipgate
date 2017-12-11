// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Squad.h"
#include "RTS_Unit.h"
#include "DrawDebugHelpers.h"

void URTS_Squad::Update(float dt)
{
	CentreOfMass = GetCentreOfMass();

	for (int32 i = 0; i < Units.Num(); ++i)
	{
		ARTS_Unit* unit = Units[i];
		FVector coherence = (CoherenceBehavior(unit) * unit->CohesionWeight);
		FVector avoid = (AvoidBehavior(unit) * unit->AvoidWeight);
		FVector seek = (SeekBehavior(unit)* unit->SeekWeight);

		FVector output = (unit->GetActorLocation() + ((coherence + avoid + seek) * VectorLengthMultiplier));
		output.Z = 0;

		unit->SetDirectionLocation(output);
	}

	//Timer
	CurrentTimer += dt;
	if (CurrentTimer <= MaxTimer)
	{
		DrawDebug();
		return;
	}
	
	CurrentTimer = 0.f;
	for (ARTS_Unit* unit : Units)
	{
		unit->OriginalTarget = ClickedTarget;
		unit->FinalTarget = ClickedTarget;
	}

	DrawDebug();
}

void URTS_Squad::DrawDebug()
{
	DrawDebugSphere(GetWorld(), CentreOfMass, 10, 12, FColor(255, 255, 0), false, 0.f, (uint8)'\000', 1.f);
}

FVector URTS_Squad::GetCentreOfMass()
{
	FVector output = FVector(0, 0, 0);

	for (int32 i = 0; i < Units.Num(); ++i)
	{
		output += Units[i]->GetActorLocation();
	}
	return output / (Units.Num());
}

FVector URTS_Squad::CoherenceBehavior(ARTS_Unit * unit)
{
	FVector output = CentreOfMass - unit->GetActorLocation();

	//Adapt weight
	float distancePercent = output.Size() / unit->CohesionDistanceClamp;
	distancePercent = FMath::Clamp(distancePercent, 0.f, 1.f);

	unit->CohesionWeight = unit->CohesionWeightClamp * distancePercent;
	output.Normalize();

	return output;
}

FVector URTS_Squad::AvoidBehavior(ARTS_Unit * unit)
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

FVector URTS_Squad::SeekBehavior(ARTS_Unit * unit)
{
	FVector output = FVector(0.0f, 0.0f, 0.0f);;

	output = unit->CurrentTarget - unit->GetActorLocation();
	output.Normalize();

	return output;
}
