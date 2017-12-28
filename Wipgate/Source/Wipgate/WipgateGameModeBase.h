// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Engine/DataTable.h"
#include "WipgateGameModeBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(WipgateGameModeBase, Log, All);

UCLASS()
class WIPGATE_API AWipgateGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

private:
	//Make sure the datatable is inheriting from FTeamRow
	UPROPERTY(EditAnywhere)
	UDataTable* m_Table;
};
