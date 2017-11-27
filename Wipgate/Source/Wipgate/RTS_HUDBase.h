// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/Image.h"
#include "Blueprint/WidgetTree.h"
#include "RTS_HUDBase.generated.h"

class ARTS_PlayerController;
class ARTS_UnitCharacter;
class UGridPanel;
class UButton;

DECLARE_LOG_CATEGORY_EXTERN(RTS_HUD_BASE_LOG, Log, All);

UCLASS(Blueprintable)
class WIPGATE_API URTS_HUDBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void UpdateSelectedUnits(const TArray<ARTS_UnitCharacter*>& SelectedUnits, bool ClearArray = true);

	template<class T>
	T* ConstructWidget(TSubclassOf<UWidget> WidgetType = T::StaticClass())
	{
		return WidgetTree->ConstructWidget<T>(WidgetType);
	}

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateSelectionBox(FVector2D Position, FVector2D Size);

	UFUNCTION(BlueprintImplementableEvent)
	void AddUnitIconToGrid(UImage* Icon);

	UFUNCTION(BlueprintImplementableEvent)
	void RemoveUnitIconFromGrid(UImage* Icon);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateUnitIcon(UImage* Icon, int GridSlotColumn, int GridSlotRow, FLinearColor IconColor);

	UFUNCTION(BlueprintImplementableEvent)
	void AddAbilityButtonToCommandCardGrid(UButton* Button, int Column, int Row, FLinearColor BackgroundColor);

	UFUNCTION(BlueprintImplementableEvent)
	void ClearAbilityButtonsFromCommandCardGrid();


	// TODO: Remove these two members carefully
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	FVector2D SelectionBoxPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	FVector2D SelectionBoxSize;

	// Unit's icon's color when at full health (blended with Low Health color when health is less than full but not empty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	FLinearColor UnitHealthColor_FullHealth = FLinearColor(0.12f, 1.0f, 0.0f, 1.0f);

	// Unit's icon's color when almost dead (blended with Full Helath color when health is less than full but not empty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	FLinearColor UnitHealthColor_LowHealth = FLinearColor(1.0f, 0.03f, 0.0f, 1.0f);

	// The color a unit's icon will be when it is dead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	FLinearColor UnitHealthColor_Dead = FLinearColor(0.22f, 0.22f, 0.22f, 1.0f);


	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	UGridPanel* SelectedUnitIconGridRef = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	UGridPanel* CommandCardGridRef = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	FMargin UnitIconPadding = FMargin(4.0f);

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	ARTS_PlayerController* PlayerController = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
	TArray<ARTS_UnitCharacter*> SelectedUnitsRef;

private:
	FIntPoint m_MaxUnitImageCount;

};
