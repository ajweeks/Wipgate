// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "RTS_HUDBase.generated.h"

class ARTS_PlayerController;
class ARTS_UnitCharacter;
class URTS_UnitIcon;
class UGridPanel;
class UButton;
class UProgressBar;

DECLARE_LOG_CATEGORY_EXTERN(RTS_HUD_BASE_LOG, Log, All);

UCLASS(Blueprintable)
class WIPGATE_API URTS_HUDBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void UpdateSelectedUnits(const TArray<ARTS_UnitCharacter*>& SelectedUnits, bool ClearArray = true);

	template<class T>
	T* ConstructWidget(TSubclassOf<UWidget> WidgetType = T::StaticClass())
	{
		return WidgetTree->ConstructWidget<T>(WidgetType);
	}

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateSelectionBox(FVector2D Position, FVector2D Size);

	// Unit icon functions
	UFUNCTION(BlueprintImplementableEvent)
		void AddUnitIconToGrid(ARTS_UnitCharacter* Unit);

	UFUNCTION(BlueprintImplementableEvent)
		void RemoveUnitIconFromGrid(int32 IconIndex);

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateUnitIconProperties(int32 IconIndex, int GridSlotColumn, int GridSlotRow, FLinearColor IconColor);

	// Ability button functions
	UFUNCTION(BlueprintImplementableEvent)
		void AddAbilityIconToCommandCardGrid(UButton* Button, UProgressBar* progressBar);

	// TODO: Make last parameter of type specialst once ported to C++
	UFUNCTION(BlueprintImplementableEvent)
		void UpdateAbilityIconProperties(UButton* Button, UProgressBar* progressBar, int Column, int Row, FLinearColor ButtonBackgroundColor, FLinearColor ProgressBarBackgroundColor, ARTS_UnitCharacter* specialist);

	UFUNCTION(BlueprintImplementableEvent)
		void ClearAbilityIconsFromCommandCardGrid();

	void OnUnitIconPressed(ARTS_UnitCharacter* unit);

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
