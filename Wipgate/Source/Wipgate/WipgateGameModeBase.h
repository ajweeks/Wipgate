// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WipgateGameModeBase.generated.h"

UCLASS()
class WIPGATE_API AWipgateGameModeBase : public AGameMode
{
	GENERATED_BODY()
	
public:
	AWipgateGameModeBase();

	virtual void BeginPlay() override;

private:


	
};
