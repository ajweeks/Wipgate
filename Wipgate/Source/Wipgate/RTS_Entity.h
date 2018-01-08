// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "RTS_Team.h"

#include "RTS_Entity.generated.h"

class UTexture2D;
class UWidgetComponent;
class UMaterial;
class UUnitEffect;
class UStaticMeshComponent;
class USoundCue;

DECLARE_LOG_CATEGORY_EXTERN(RTS_ENTITY_LOG, Log, All);

//TODO: Logging warning when unit is structure (EntityType)
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

	UFUNCTION(BlueprintCallable)
	FVector GetGroundLocation();

	UFUNCTION(BlueprintCallable, Category = "Team")
	virtual void SetTeamMaterial();

	UFUNCTION(BlueprintCallable)
	void PostInitialize();

	/* --- Unit effect functions --- */
	UFUNCTION(BlueprintGetter, Category = "Effects")
		TArray<UUnitEffect*> GetUnitEffects() const;
	UFUNCTION(BlueprintCallable, Category = "Effects")
		bool HasEffectWithTag(FName tag);
	UFUNCTION(BlueprintCallable, Category = "Effects")
		void RemoveUnitEffectWithTag(FName tag);
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
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsAlive();

	UFUNCTION(BlueprintCallable)
	void AddToLumaSaturation(int32 LumaToAdd);

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
	bool RenderFlockingDebugInfo = false;

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
	UPROPERTY(BlueprintReadWrite)
		bool m_IsAttackOnCooldown = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FDefenceStat BaseDefenceStats;
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FDefenceStat CurrentDefenceStats;
	
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FLumaStat CurrentLumaStats;

	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int Health = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FVisionStat BaseVisionStats;
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FVisionStat CurrentVisionStats;

	//TEAM
	UPROPERTY(BlueprintReadWrite, Category = "Team")
	URTS_Team* Team;

	UPROPERTY(BlueprintReadWrite)
	TArray<UStaticMeshComponent*> DebugMeshes;

	UPROPERTY(EditAnywhere, Category = "Team")
	ETeamAlignment Alignment = ETeamAlignment::E_PLAYER;

	//TYPE
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEntityType EntityType = EEntityType::E_RANGED;

	UPROPERTY(BlueprintReadWrite)
	TArray<UUnitEffect*> UnitEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 IconIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconTexture;

	const int NUM_ABILITIES = 3;
	bool ShowingAbilityIcons = false;

	//Sounds
	UPROPERTY(EditAnywhere)
		USoundCue* Sound;

	//Death
	UPROPERTY(BlueprintReadOnly)
		FVector LocationOfDeath;

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
