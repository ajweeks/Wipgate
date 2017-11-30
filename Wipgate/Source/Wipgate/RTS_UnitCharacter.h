// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RTS_UnitCoreComponent.h"
#include "UnitEffect.h"
#include "AbilityIcon.h"
#include "RTS_UnitCharacter.generated.h"

class UImage;
class AbilityIcon;

DECLARE_LOG_CATEGORY_EXTERN(RTS_Unit_Log, Log, All);

UCLASS()
class WIPGATE_API ARTS_UnitCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARTS_UnitCharacter();

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintNativeEvent)
	void SetSelected(bool selected);
	virtual void SetSelected_Implementation(bool selected);

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintGetter)
	bool IsSelected() const;
	UFUNCTION(BlueprintGetter, Category = "Effects")
	TArray<UUnitEffect*> GetUnitEffects() const;
	UFUNCTION(BlueprintCallable, Category = "Effects")
	void AddUnitEffect(UUnitEffect* effect);
	UFUNCTION(BlueprintCallable, Category = "Effects")
	void RemoveUnitEffect(UUnitEffect* effect);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DisableDebug();

public:
	/* Public blueprint editable variables */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	FVector SelectionHitBox = FVector(30.0f, 30.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	float SelectionBrightness = 5.f;

	UPROPERTY(BlueprintReadWrite)
	URTS_UnitCoreComponent* UnitCoreComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool ShowRange = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float RangeHeight = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool ShowUnitStats = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool ShowSelectionBox = false;

	UPROPERTY(BlueprintReadWrite)
	TArray<UStaticMeshComponent*> DebugMeshes;

	UPROPERTY(BlueprintReadWrite)
	TArray<UUnitEffect*> UnitEffects;

	UImage* Icon = nullptr;

	const int NUM_ABILITIES = 3;
	TArray<FAbilityIcon> AbilityIcons; // Set to nullptrs when not visible

private:
	/* private functions */
	void ApplyEffectLinear(UUnitEffect* effect);
	void ApplyEffectOnce (UUnitEffect* effect);

	/* private members */
	UPROPERTY(VisibleAnywhere, Category = "Selection")
	bool Selected;
	
	const int EFFECT_INTERVAL = 1;
};
