// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnitEffect.h"
#include "GeneralFunctionLibrary_CPP.generated.h"

class APlayerController;

UCLASS()
class WIPGATE_API UGeneralFunctionLibrary_CPP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

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
	
	// Math functions
	UFUNCTION(BlueprintPure, Category = "Project")
		static void FVector2DMinMax(FVector2D& vec1, FVector2D& vec2);

	UFUNCTION(BlueprintPure, Category = "Project")
		static void FVectorMinMax(FVector& vec1, FVector& vec2);

	UFUNCTION(BlueprintPure, Category = "Project")
		static bool PointInBounds2D(FVector2D point, FVector2D boundsMin, FVector2D boundsMax);

	UFUNCTION(BlueprintPure, Category = "Project")
		static FVector2D GetNormalizedMousePosition(APlayerController* playerController);

	UFUNCTION(BlueprintPure, Category = "Project")
		static FVector2D GetMousePositionVector2D(APlayerController* playerController);
};

// Global C++ only functions
void PrintStringToScreen(FString text);
