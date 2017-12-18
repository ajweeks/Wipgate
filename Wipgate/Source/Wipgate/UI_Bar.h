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

	UFUNCTION(BlueprintCallable)
	FLinearColor GetTeamColor();

	// When true, this bar will not update it's value
	// (used to freeze commander's bar while leaping)
	UPROPERTY(BlueprintReadWrite)
	bool Frozen = false;

private:
	ARTS_Entity* EntityRef;
	
	float m_LastHealthBarPercent = 0.0f;
	FLinearColor m_FrozenColor;

	// How much to desaturate the team color in range [0..1] (0 = no desaturation, 1 = fully gray)
	float m_FrozenColorDesaturationAmount = 0.7f;

};
