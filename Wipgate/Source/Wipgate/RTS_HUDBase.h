// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "RTS_HUDBase.generated.h"

class ARTS_PlayerController;
class ARTS_Entity;
class ARTS_Unit;
class ARTS_Specialist;
class UGridPanel;
class UButton;
class UProgressBar;

DECLARE_LOG_CATEGORY_EXTERN(RTS_HUD_BASE_LOG, Log, All);

UCLASS(Blueprintable)
class WIPGATE_API URTS_HUDBase : public UUserWidget
{
	GENERATED_BODY()

public:
	URTS_HUDBase(const FObjectInitializer& ObjectInitializer);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void UpdateSelectedEntities(const TArray<ARTS_Entity*>& SelectedEntities, bool ClearArray = true);

	template<class T>
	T* ConstructWidget(TSubclassOf<UWidget> WidgetType = T::StaticClass())
	{
		return WidgetTree->ConstructWidget<T>(WidgetType);
	}

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateSelectionBox(FVector2D Position, FVector2D Size);


	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void SetVisible(bool Visible);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void ToggleVisible();

	// Entity icon functions
	UFUNCTION(BlueprintImplementableEvent)
		void AddEntityIconToGrid(ARTS_Entity* Entity);

	UFUNCTION(BlueprintImplementableEvent)
		void RemoveEntityIconFromGrid(int32 IconIndex);

	UFUNCTION(BlueprintImplementableEvent)
		void ClearEntityIconsFromGrid();

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateEntityIconProperties(int32 IconIndex, int GridSlotColumn, int GridSlotRow, FLinearColor IconColor);


	// Ability icon functions
	UFUNCTION(BlueprintImplementableEvent)
		void AddAbilityIconToCommandCardGrid(ARTS_Specialist* Specialist);

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateAbilityIconProperties(int32 Index, int32 Column, int32 Row, FLinearColor ButtonBackgroundColor, FLinearColor ProgressBarBackgroundColor);

	UFUNCTION(BlueprintImplementableEvent)
		void ClearAbilityIconsFromCommandCardGrid();


	// Entity stats functions
	UFUNCTION(BlueprintImplementableEvent)
		void ShowSelectedUnitStats(ARTS_Entity* Entity);

	UFUNCTION(BlueprintImplementableEvent)
		void HideSelectedUnitStats();


	// Selection group icon functions
	/* 
		This function should be called once at startup to create all necessary buttons
		All buttons created will be invisible (and unclickable). Call ShowSelectionGroupIcon
		to make button visible and clickable
	*/ 
	UFUNCTION(BlueprintImplementableEvent)
		void AddSelectionGroupIconsToGrid(int32 Count);

	UFUNCTION(BlueprintImplementableEvent)
		void ShowSelectionGroupIcon(int32 Index);

	UFUNCTION(BlueprintImplementableEvent)
		void HideSelectionGroupIcon(int32 Index);


	UFUNCTION(BlueprintCallable)
		void DeselectEntity(ARTS_Entity* Entity);


	UFUNCTION(BlueprintImplementableEvent)
		void UpdateCurrencyAmount(int32 CurrencyAmount);

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateLumaAmount(int32 LumaAmount);


	// Entity's icon's color when at full health (blended with Low Health color when health is less than full but not empty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
		FLinearColor EntityIconHealthColor_FullHealth = FLinearColor(0.12f, 1.0f, 0.0f, 1.0f);

	// Entity's icon's color when almost dead (blended with Full Helath color when health is less than full but not empty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
		FLinearColor EntityIconHealthColor_LowHealth = FLinearColor(1.0f, 0.03f, 0.0f, 1.0f);

	// Entity's icon's color when dead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
		FLinearColor EntityIconHealthColor_Dead = FLinearColor(0.22f, 0.22f, 0.22f, 1.0f);


	UPROPERTY(BlueprintReadWrite, Category = "Misc")
		UGridPanel* SelectedEntityIconGridRef = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
		UGridPanel* CommandCardGridRef = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
		FMargin EntityIconPadding = FMargin(4.0f);

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
		ARTS_PlayerController* PlayerController = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Misc")
		TArray<ARTS_Entity*> SelectedEntitiesRef;

private:
	FIntPoint m_MaxEntityImageCount;

};
