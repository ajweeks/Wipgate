// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTS_Unit.h"
#include "RTS_Specialist.generated.h"

class AAbility;

UCLASS()
class WIPGATE_API ARTS_Specialist : public ARTS_Unit
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadWrite, Category = "Abilities")
	AAbility* AbilityActive;
	UPROPERTY(BlueprintReadWrite, Category = "Abilities")
	AAbility* AbilityConstruct;
	UPROPERTY(BlueprintReadWrite, Category = "Abilities")
	AAbility* AbilityPassive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<AAbility> AbilityActiveClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<AAbility> AbilityConstructClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<AAbility> AbilityPassiveClass;
	
};
