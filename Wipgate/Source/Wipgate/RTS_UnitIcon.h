// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "RTS_UnitIcon.generated.h"

class ARTS_UnitCharacter;

DECLARE_LOG_CATEGORY_EXTERN(RTS_UnitIconLog, Log, All);

UCLASS()
class WIPGATE_API URTS_UnitIcon : public UButton
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
		ARTS_UnitCharacter* Unit;

};
