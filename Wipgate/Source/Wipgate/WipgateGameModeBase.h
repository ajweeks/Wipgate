// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "WipgateGameModeBase.generated.h"

UCLASS()
class WIPGATE_API AWipgateGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AWipgateGameModeBase();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "My category")
	TSubclassOf<UUserWidget> Main_UI_HUD;

private:


	UUserWidget* CurrentWidget;
	
};
