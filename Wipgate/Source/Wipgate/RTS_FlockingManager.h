// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTS_Unit.h"
#include "GameFramework/Actor.h"

#include "RTS_FlockingManager.generated.h"


UCLASS()
class WIPGATE_API ARTS_FlockingManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARTS_FlockingManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Functions
	FVector GetCentreOfMass();
	FVector CoherenceBehavior(ARTS_Unit* unit);
	FVector AvoidBehavior(ARTS_Unit* unit);
	FVector SeekBehavior(ARTS_Unit* unit);

	//Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Attributes")
		FVector SpawnBoundsStart = FVector(-1000, 1000, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Attributes")
		FVector SpawnBoundsEnd = FVector(1000, 1000, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Attributes")
		int32 NumberOfUnits = 20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flock attributes")
		float UnitAvoidRange = 75.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flock attributes")
		float VectorLengthMultiplier = 5.f;
	
	//Variables
	UPROPERTY(BlueprintReadWrite)
		FVector CentreOfMass;

	UPROPERTY(BlueprintReadWrite)
		TArray<ARTS_Unit*> Units;
	
};
