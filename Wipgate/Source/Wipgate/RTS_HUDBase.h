// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RTS_HUDBase.generated.h"

class ARTS_PlayerController;

UCLASS(Blueprintable)
class WIPGATE_API URTS_HUDBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateSelectionBox(FVector2D position, FVector2D size);

	// TODO: Remove these carefully
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	FVector2D SelectionBoxPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	FVector2D SelectionBoxSize;

};
