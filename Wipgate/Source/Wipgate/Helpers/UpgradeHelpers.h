// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EntityHelpers.h"
//#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "UpgradeHelpers.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	E_FLAT 				UMETA(DisplayName = "Flat upgrade"),
	E_PERCENTUAL 		UMETA(DisplayName = "Percentual upgrade")
};

//MAKE SURE FIRST AND LAST ELEMENT ALWAYS STAY THE SAME
UENUM(BlueprintType)
enum class EUpgradeStat : uint8
{
	E_SPEED				UMETA(DisplayName = "Speed"),
	E_DAMAGE			UMETA(DisplayName = "Damage"),
	E_RATEOFFIRE		UMETA(DisplayName = "Rate of Fire"),
	E_RANGE				UMETA(DisplayName = "Range"),
	E_ARMOR				UMETA(DisplayName = "Armor"),
	E_HEALTH			UMETA(DisplayName = "Health"),
	E_VISION			UMETA(DisplayName = "Vision")
};

USTRUCT(BlueprintType)
struct FUpgrade
{
	GENERATED_USTRUCT_BODY()

		//The effect it has on the stat its altering
		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Effect = 0.f;

	//Type of the upgrade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EUpgradeType Type = EUpgradeType::E_FLAT;

	//Stat upgrade affects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EUpgradeStat Stat = EUpgradeStat::E_ARMOR;

	//Unit it affects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EEntityType AffectedType;
};

USTRUCT(BlueprintType)
struct FUpgradeRow : public FTableRowBase
{
	FUpgradeRow() {}
public:
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Title = "Title";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Description = "Description";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FLinearColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FUpgrade> Upgrades;
};

USTRUCT(BlueprintType)
struct FEnemyUpgradeRow : public FTableRowBase
{
	FEnemyUpgradeRow() {}
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FUpgrade> Upgrades;
};