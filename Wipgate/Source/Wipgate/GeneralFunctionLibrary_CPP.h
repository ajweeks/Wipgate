// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnitEffect.h"
#include "GeneralFunctionLibrary_CPP.generated.h"

UCLASS()
class WIPGATE_API UGeneralFunctionLibrary_CPP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "Ability Creation Functions")
	static UUnitEffect* CreateUnitEffect(UObject * outer, const EUnitEffectStat stat, const EUnitEffectType type,
			const float delay, const int magnitude, const int duration);
	
	//Returns the project version set in the 'Project Settings' > 'Version' section of the editor
	UFUNCTION(BlueprintPure, Category = "Project")
		static FString GetProjectVersion();

	//Returns the project name set in the 'Project Settings' > 'Name' section of the editor
	UFUNCTION(BlueprintPure, Category = "Project")
		static FString GetProjectName();

	//Returns the project name set in the 'Project Settings' > 'Description' section of the editor
	UFUNCTION(BlueprintPure, Category = "Project")
		static FString GetProjectDescription();
	
};

// Global C++ only functions
void PrintStringToScreen(FString text);
