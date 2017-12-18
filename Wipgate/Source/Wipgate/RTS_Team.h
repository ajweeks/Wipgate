// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WipgateGameModeBase.h"

#include "RTS_Team.generated.h"

UCLASS()
class WIPGATE_API URTS_Team : public UObject
{
	GENERATED_BODY()
	
	public:
		UPROPERTY(BlueprintReadOnly)
		FLinearColor Color = FLinearColor(1, 1, 1, 1);
		UPROPERTY(BlueprintReadOnly)
		ETeamAlignment Alignment = ETeamAlignment::E_NEUTRAL_AI;
};
