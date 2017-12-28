// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilityIconBase.generated.h"

UCLASS()
class WIPGATE_API UAbilityIconBase : public UUserWidget
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintImplementableEvent)
		void OnAbilityActivate();
	
};
