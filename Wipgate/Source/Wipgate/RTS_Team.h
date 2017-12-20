// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WipgateGameModeBase.h"
#include "UnitEffect.h"
#include "RTS_Team.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(RTS_TEAM_LOG, Log, All);

class ARTS_Entity;

UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	E_FLAT 				UMETA(DisplayName = "Flat upgrade"),
	E_PERCENTUAL 		UMETA(DisplayName = "Percentual upgrade")
};

USTRUCT(BlueprintType)
struct FUpgrade
{
	GENERATED_USTRUCT_BODY()

	//The effect it has on the stat its altering
	UPROPERTY(BlueprintReadWrite)
	float Effect = 0.f;

	//Type of the upgrade
	UPROPERTY(BlueprintReadWrite)
	EUpgradeType Type = EUpgradeType::E_FLAT;

	//Stat upgrade affects
	UPROPERTY(BlueprintReadWrite)
	EUnitEffectStat Stat = EUnitEffectStat::HEALING;
};

UCLASS()
class WIPGATE_API URTS_Team : public UObject
{
	GENERATED_BODY()
	
	public:
		/* Functions */
		UFUNCTION(BlueprintCallable)
		void AddUpgrade(FUpgrade upgrade);
		UFUNCTION(BlueprintCallable)
		void CalculateUpgradeEffects();

		/* Variables */
		UPROPERTY(BlueprintReadWrite)
		TArray<ARTS_Entity*> Entities;
		UPROPERTY(BlueprintReadOnly)
		FLinearColor Color = FLinearColor(1, 1, 1, 1);
		UPROPERTY(BlueprintReadOnly)
		ETeamAlignment Alignment = ETeamAlignment::E_NEUTRAL_AI;
		UPROPERTY(BlueprintReadOnly)
		TArray<FUpgrade> Upgrades;
};
