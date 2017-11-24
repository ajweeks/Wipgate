// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/Image.h"
#include "RTS_HUDBase.generated.h"

class ARTS_PlayerController;
class ARTS_UnitCharacter;
class UGridPanel;

DECLARE_LOG_CATEGORY_EXTERN(RTS_HUD_BASE_LOG, Log, All);

UCLASS(Blueprintable)
class WIPGATE_API URTS_HUDBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void UpdateSelectedUnits(const TArray<ARTS_UnitCharacter*>& SelectedUnits);

	//UFUNCTION(BlueprintImplementableEvent)
	//void UpdateSelectedUnitIcons(const TArray<FSelectedUnitIconInfo>& SelectedUnitInfos);

	//UFUNCTION(BlueprintImplementableEvent)
	//void UpdateSelectedUnitIconColor(UImage* Icon, FLinearColor Color);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateSelectionBox(FVector2D position, FVector2D size);

	// TODO: Remove these carefully
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	FVector2D SelectionBoxPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	FVector2D SelectionBoxSize;

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	UGridPanel* SelectedUnitIconsHolderRef = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	ARTS_PlayerController* PlayerController = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	TArray<UImage*> SelectedUnitIcons;

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	TArray<ARTS_UnitCharacter*> SelectedUnitsRef;

	//UPROPERTY(BlueprintReadWrite, Category = "Misc")
	//UImage* SelectedUnitsBackgroundImageRef = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	FVector2D SelectedUnitIconSize = FVector2D(32.0f, 32.0f);

	// Unit's icon's color when at full health (blended with Low Health color when health is less than full but not empty)
	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	FLinearColor UnitHealthColor_FullHealth;

	// Unit's icon's color when almost dead (blended with Full Helath color when health is less than full but not empty)
	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	FLinearColor UnitHealthColor_LowHealth;

	// The color a unit's icon will be when it is dead
	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	FLinearColor UnitHealthColor_Dead;

private:
	FIntPoint m_MaxUnitImageCount;

};
