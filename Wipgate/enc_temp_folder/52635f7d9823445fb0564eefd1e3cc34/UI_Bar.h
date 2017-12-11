// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_Bar.generated.h"

class ARTS_Entity;

UCLASS()
class WIPGATE_API UUI_Bar : public UUserWidget
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable)
	void Initialize(AActor* Owner);

	UFUNCTION(BlueprintCallable)
	float GetHealthBarPercent();

private:
	ARTS_Entity* EntityRef;
	

};
