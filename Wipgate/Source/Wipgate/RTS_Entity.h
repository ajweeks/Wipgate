// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"

#include "AbilityIcon.h"

#include "RTS_Entity.generated.h"

class UImage;
class UWidgetComponent;
class UMaterial;
class UUnitEffect;
class UStaticMeshComponent;
struct FAbilityIcon;

DECLARE_LOG_CATEGORY_EXTERN(RTS_ENTITY_LOG, Log, All);

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
	int Health = 100;
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
enum class EAlignment : uint8
{
	E_FRIENDLY 		UMETA(DisplayName = "Friendly"),
	E_NEUTRAL 		UMETA(DisplayName = "Neutral"),
	E_ENEMY 		UMETA(DisplayName = "Enemy"),
};

USTRUCT(BlueprintType)
struct FTeamRow : public FTableRowBase
{
	FTeamRow(){}
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(1, 1, 1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAlignment Alignment = EAlignment::E_NEUTRAL;
};

USTRUCT(BlueprintType)
struct FTeam
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name = "Team";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(1, 1, 1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAlignment Alignment = EAlignment::E_NEUTRAL;
};

UENUM(BlueprintType)
enum class EEntityType : uint8
{
	E_UNIT 				UMETA(DisplayName = "Unit Character"),
	E_STRUCTURE 		UMETA(DisplayName = "Static Structure")
};



UCLASS()
class WIPGATE_API ARTS_Entity : public ACharacter
{
	GENERATED_BODY()

public:
	ARTS_Entity();

public:
	UFUNCTION(BlueprintCallable)
	void SetSelected(bool selected);

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintGetter, Category="Selection")
	bool IsSelected() const;

	UFUNCTION(BlueprintCallable, Category = "Team")
	virtual void SetTeamMaterial();

	UFUNCTION(BlueprintGetter, Category = "Effects")
	TArray<UUnitEffect*> GetUnitEffects() const;
	UFUNCTION(BlueprintCallable, Category = "Effects")
	void AddUnitEffect(UUnitEffect* effect);
	UFUNCTION(BlueprintCallable, Category = "Effects")
	void RemoveUnitEffect(UUnitEffect* effect);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DisableDebug();
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetRangeDebug();

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool ApplyDamage(int damage, bool armor);
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyHealing(int healing);
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void Kill();

public:
	/* Public blueprint editable variables */

	//SELECTION
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	FVector SelectionHitBox = FVector(30.0f, 30.0f, 100.0f);

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Selection")
	UStaticMeshComponent* SelectionStaticMeshComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	float SelectionBrightness = 5.0f;

	//UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UWidgetComponent* BarWidget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UStaticMeshComponent* MinimapIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FName MinimapColorParameterName = "None";

	FRotator BarRotation;

	//DEBUG
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool ShowRange = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float RangeHeight = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool ShowUnitStats = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool ShowSelectionBox = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	UMaterial* RangeMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	UStaticMesh* RangeMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	FName RangeColorParameterName = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	FLinearColor RangeInnerVisionColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	FLinearColor RangeAttackColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	FLinearColor RangeOuterVisionColor;

	//STATS
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FMovementStat BaseMovementStats;
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FMovementStat CurrentMovementStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FAttackStat BaseAttackStats;
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FAttackStat CurrentAttackStats;

	UPROPERTY(BlueprintReadWrite)
	float TimerRateOfFire = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FDefenceStat BaseDefenceStats;
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FDefenceStat CurrentDefenceStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FVisionStat BaseVisionStats;
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FVisionStat CurrentVisionStats;

	//TEAM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FDataTableRowHandle TeamRow;

	UPROPERTY(BlueprintReadWrite, Category = "Team")
	FTeam Team;

	UPROPERTY(BlueprintReadWrite)
	TArray<UStaticMeshComponent*> DebugMeshes;

	UPROPERTY(BlueprintReadWrite)
	TArray<UUnitEffect*> UnitEffects;

	UImage* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 IconIndex = -1;

	const int NUM_ABILITIES = 3;
	TArray<FAbilityIcon> AbilityIcons; // Set to nullptrs when not visible

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	/* private functions */
	void ApplyEffectLinear(UUnitEffect* effect);
	void ApplyEffectOnce(UUnitEffect* effect);

	/* private members */
	UPROPERTY(VisibleAnywhere, Category = "Selection")
	bool Selected;

	const int EFFECT_INTERVAL = 1;
	
};
