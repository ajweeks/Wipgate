// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTS_Entity.h"
#include "RTS_Unit.generated.h"

class USkeletalMeshComponent;

UCLASS()
class WIPGATE_API ARTS_Unit : public ARTS_Entity
{
	GENERATED_BODY()
	
public:
	ARTS_Unit();

	UFUNCTION(BlueprintCallable, Category = "Team")
	virtual void SetTeamMaterial() override;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void Kill() override;

	void SetDirectionLocation(FVector location);

	//Properties
	UPROPERTY(BlueprintReadWrite)
		FVector CurrentTarget;
	UPROPERTY(BlueprintReadWrite)
		FVector OriginalTarget;
	UPROPERTY(BlueprintReadWrite)
		FVector FinalTarget;

	UPROPERTY(EditAnywhere, Category = "RuleWeights", BlueprintReadWrite)
		float CohesionWeight = 0.25f;
	UPROPERTY(EditAnywhere, Category = "RuleWeights", BlueprintReadWrite)
		float CohesionDistanceClamp = 500.f;
	UPROPERTY(EditAnywhere, Category = "RuleWeights", BlueprintReadWrite)
		float CohesionWeightClamp = 1.f;
	UPROPERTY(EditAnywhere, Category = "RuleWeights", BlueprintReadWrite)
		float AvoidWeight = 0.5f;
	UPROPERTY(EditAnywhere, Category = "RuleWeights", BlueprintReadWrite)
		float AvoidDistanceClamp = 500.f;
	UPROPERTY(EditAnywhere, Category = "RuleWeights", BlueprintReadWrite)
		float AvoidWeightClamp = 1.f;
	UPROPERTY(EditAnywhere, Category = "RuleWeights", BlueprintReadWrite)
		float AlignWeight = 0.3f;
	UPROPERTY(EditAnywhere, Category = "RuleWeights", BlueprintReadWrite)
		float AlignWeightClamp = 1.f;
	UPROPERTY(EditAnywhere, Category = "RuleWeights", BlueprintReadWrite)
		float SeekWeight = 4.0f;
	UPROPERTY(EditAnywhere, Category = "RuleWeights", BlueprintReadWrite)
		float SeekWeightClamp = 1.f;

	// When true, this unit's stats can not be changed, and it can not be targeted
	UPROPERTY(BlueprintReadWrite)
		bool Immaterial = false;

private:
	
};
