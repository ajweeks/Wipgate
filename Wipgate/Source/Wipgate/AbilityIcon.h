// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilityIcon.generated.h"

class UButton;
class UProgressBar;

USTRUCT()
struct WIPGATE_API FAbilityIcon
{
	GENERATED_BODY()

	UButton* Button;
	UProgressBar* ProgressBar;
};
