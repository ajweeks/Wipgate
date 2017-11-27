// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_HUDBase.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/UMG/Public/Components/PanelSlot.h"
#include "Runtime/UMG/Public/Components/GridSlot.h"
#include "Runtime/UMG/Public/Components/GridPanel.h"
#include "Runtime/UMG/Public/Blueprint/WidgetTree.h"
#include "RTS_UnitCharacter.h"

DEFINE_LOG_CATEGORY(RTS_HUD_BASE_LOG);

void URTS_HUDBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::Tick(MyGeometry, InDeltaTime);

	if (SelectedUnitIconSize == FVector2D::ZeroVector)
	{
		UE_LOG(RTS_HUD_BASE_LOG, Error, TEXT("Selected unit icon size not set!"));
		return;
	}

	if (!SelectedUnitIconGridRef)
	{
		UE_LOG(RTS_HUD_BASE_LOG, Error, TEXT("Selected unit icons grid not set! (Should be set in HUD BP's constructor)"));
		return;
	}

	if (SelectedUnitsRef.Num() > 0)
	{
		FIntPoint viewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
		FVector2D selectedUnitsBGImageAbsoluteSize = SelectedUnitIconGridRef->GetCachedGeometry().GetAbsoluteSize();
		selectedUnitsBGImageAbsoluteSize.X = FMath::Clamp(selectedUnitsBGImageAbsoluteSize.X, 0.0f, (float)viewportSize.X);
		selectedUnitsBGImageAbsoluteSize.Y = FMath::Clamp(selectedUnitsBGImageAbsoluteSize.Y, 0.0f, (float)viewportSize.Y);

		FVector2D iconSize = SelectedUnitsRef[0]->Icon->GetCachedGeometry().GetAbsoluteSize();
		iconSize.X += UnitIconPadding.Left + UnitIconPadding.Right;
		iconSize.Y += UnitIconPadding.Top + UnitIconPadding.Bottom;

		FVector2D maxSelectedUnitImageCountF = (selectedUnitsBGImageAbsoluteSize / iconSize);
		FIntPoint maxSelectedUnitImageCount = FIntPoint(
			FMath::TruncToInt(maxSelectedUnitImageCountF.X) - 1, 
			FMath::TruncToInt(maxSelectedUnitImageCountF.Y) - 1);

		m_MaxUnitImageCount = maxSelectedUnitImageCount;
		UpdateSelectedUnits(SelectedUnitsRef, false);
	}
}

void URTS_HUDBase::UpdateSelectedUnits(const TArray<ARTS_UnitCharacter*>& SelectedUnits, bool ClearArray)
{
	int32 newSelectedUnitCount = SelectedUnits.Num();
	int32 newSelectedUnitIconCount = FMath::Min(newSelectedUnitCount, m_MaxUnitImageCount.X * m_MaxUnitImageCount.Y);

	if (ClearArray)
	{
		for (auto oldSelectedUnit : SelectedUnitsRef)
		{
			RemoveUnitIconFromGrid(oldSelectedUnit->Icon);
			oldSelectedUnit->Icon = nullptr;
		}

		for (auto newSelectedUnit : SelectedUnits)
		{
			newSelectedUnit->Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			AddUnitIconToGrid(newSelectedUnit->Icon);
		}

		SelectedUnitsRef = SelectedUnits;
	}

	// TODO: Handle == 1 seperately
	if (newSelectedUnitIconCount >= 1)
	{
		for (int32 i = 0; i < newSelectedUnitIconCount; ++i)
		{
			ARTS_UnitCharacter* unit = SelectedUnitsRef[i];
			checkSlow(unit);
			URTS_UnitCoreComponent* unitCoreComponent = unit->m_UnitCoreComponent;
			checkSlow(unitCoreComponent);

			int col = i % m_MaxUnitImageCount.X;
			int row = i / m_MaxUnitImageCount.X;
			float unitHealthNorm = (float)unitCoreComponent->CurrentHealth / (float)unitCoreComponent->BaseHealth;
			FLinearColor color;
			if (unitHealthNorm > 0.0f)
			{
				color = FMath::Lerp(UnitHealthColor_LowHealth, UnitHealthColor_FullHealth, unitHealthNorm);
			}
			else
			{
				color = UnitHealthColor_Dead;
			}

			UpdateUnitIcon(unit->Icon, col, row, color);
		}
	}
	else if (newSelectedUnitIconCount == 1)
	{
		// TODO: Change if statement to `> 1` and show individual unit's stats
	}
}
