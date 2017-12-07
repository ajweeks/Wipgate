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

private:
	
};
