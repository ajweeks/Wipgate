// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Engine/DataTable.h"
#include "WipgateGameModeBase.generated.h"

<<<<<<< HEAD
USTRUCT(BlueprintType)
struct FMovementStat
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Speed = 500;
};

USTRUCT(BlueprintType)
struct FAttackStat
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int Damage = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AttackDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RateOfFire = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Range = 250.0f;
};

USTRUCT(BlueprintType)
struct FDefenceStat
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int Armor = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int MaxHealth = 100;
};

USTRUCT(BlueprintType)
struct FVisionStat
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float OuterRange = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float InnerRange = 500;
};

UENUM(BlueprintType)
enum class ETeamAlignment : uint8
{
	E_PLAYER 			UMETA(DisplayName = "Player"),
	E_NEUTRAL_AI 		UMETA(DisplayName = "Neutral AI"),
	E_AGGRESSIVE_AI 		UMETA(DisplayName = "Aggressive AI")
};

UENUM(BlueprintType)
enum class ERelativeAlignment : uint8
{
	E_FRIENDLY 			UMETA(DisplayName = "Friendly"),
	E_ENEMY				UMETA(DisplayName = "Enemy")
};

UENUM(BlueprintType)
enum class EEntityType : uint8
{
	E_STRUCTURE 		UMETA(DisplayName = "Static Structure"),
	E_RANGED			UMETA(DisplayName = "Ranged unit"),
	E_MELEE				UMETA(DisplayName = "Melee unit"),
	E_SPECIALIST		UMETA(DisplayName = "Specialist unit")
};

USTRUCT(BlueprintType)
struct FTeamRow : public FTableRowBase
{
	FTeamRow() {}
public:
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FLinearColor Color = FLinearColor(1, 1, 1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ETeamAlignment Alignment = ETeamAlignment::E_NEUTRAL_AI;
};

=======
>>>>>>> 215ba312b56e313535b16f65cbcd5cbc9d2ade1e
DECLARE_LOG_CATEGORY_EXTERN(WipgateGameModeBase, Log, All);

UCLASS()
class WIPGATE_API AWipgateGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

private:
	//Make sure the datatable is inheriting from FTeamRow
	UPROPERTY(EditAnywhere)
	UDataTable* m_Table;
};
