// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RTS_UnitCoreComponent.h"
#include "UnitEffect.h"
#include "RTS_UnitCharacter.generated.h"

class UImage;

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

public:
	/* Public blueprint editable variables */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	FVector SelectionHitBox = FVector(30.0f, 30.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	bool ShowSelectionBox_DEBUG;

	UPROPERTY(BlueprintReadWrite)
	URTS_UnitCoreComponent* m_UnitCoreComponent = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TArray<UUnitEffect*> m_UnitEffects;

	UImage* Icon = nullptr;
	
private:
	/* private functions */
	void ApplyEffectLinear(UUnitEffect* effect);
	void ApplyEffectOnce (UUnitEffect* effect);

	/* private members */
	UPROPERTY(VisibleAnywhere, Category = "Selection")
	bool Selected;
	
	const int EFFECT_INTERVAL = 1;
};
