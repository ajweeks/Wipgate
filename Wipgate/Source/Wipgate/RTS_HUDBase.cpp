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

	//if (!SelectedUnitsBackgroundImageRef)
	//{
	//	UE_LOG(RTS_HUD_BASE_LOG, Error, TEXT("Unit selection background image not set!"));
	//	return;
	//}

	if (SelectedUnitIconSize == FVector2D::ZeroVector)
	{
		UE_LOG(RTS_HUD_BASE_LOG, Error, TEXT("Selected unit icon size not set!"));
		return;
	}

	if (!SelectedUnitIconsHolderRef)
	{
		UE_LOG(RTS_HUD_BASE_LOG, Error, TEXT("Selected unit icons holder not set!"));
		return;
	}

	// ?
	SelectedUnitIconsHolderRef->ForceLayoutPrepass();

	FIntPoint viewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
	FVector2D selectedUnitsBGImageAbsoluteSize = SelectedUnitIconsHolderRef->GetCachedGeometry().GetAbsoluteSize();
	selectedUnitsBGImageAbsoluteSize.X = FMath::Clamp(selectedUnitsBGImageAbsoluteSize.X, 0.0f, (float)viewportSize.X);
	selectedUnitsBGImageAbsoluteSize.Y = FMath::Clamp(selectedUnitsBGImageAbsoluteSize.Y, 0.0f, (float)viewportSize.Y);

	FVector2D maxSelectedUnitImageCountF = (selectedUnitsBGImageAbsoluteSize / SelectedUnitIconSize);
	FIntPoint maxSelectedUnitImageCount = FIntPoint(FMath::TruncToInt(maxSelectedUnitImageCountF.X), FMath::TruncToInt(maxSelectedUnitImageCountF.Y));

	//if (maxSelectedUnitImageCount != m_MaxUnitImageCount)
	//{
		m_MaxUnitImageCount = maxSelectedUnitImageCount;
		UpdateSelectedUnits(SelectedUnitsRef);
	//}
}

void URTS_HUDBase::UpdateSelectedUnits(const TArray<ARTS_UnitCharacter*>& SelectedUnits)
{
	SelectedUnitsRef = SelectedUnits;

	int32 prevSelectedUnitCount = SelectedUnitIcons.Num();
	int32 newSelectedUnitCount = SelectedUnitsRef.Num();
	if (prevSelectedUnitCount > newSelectedUnitCount)
	{
		for (int32 i = newSelectedUnitCount; i < prevSelectedUnitCount; ++i)
		{
			// Remove now de-selected units' icons from UI
			SelectedUnitIconsHolderRef->RemoveChildAt(i);
		}
	}

	int32 newSelectedUnitIconCount = FMath::Min(newSelectedUnitCount, m_MaxUnitImageCount.X * m_MaxUnitImageCount.Y);

	SelectedUnitIcons.SetNum(newSelectedUnitIconCount);

	for (int32 i = 0; i < SelectedUnitIcons.Num(); ++i)
	{
		//if (!IsValid(SelectedUnitIcons[i]))
		{
			SelectedUnitIcons[i] = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			SelectedUnitIconsHolderRef->AddChildToGrid(SelectedUnitIcons[i])->Column = i % 3;
		}
	}

	if (newSelectedUnitIconCount > 1)
	{
		for (int32 i = 0; i < newSelectedUnitIconCount; ++i)
		{
			ARTS_UnitCharacter* selectedUnit = SelectedUnitsRef.GetData()[i];

			UImage* icon = SelectedUnitIcons.GetData()[i];
			//checkSlow(icon);

			UGridSlot* gridSlot = Cast<UGridSlot>(SelectedUnitIcons.GetData()[i]->Slot);
			gridSlot->Column = i % m_MaxUnitImageCount.X;
			gridSlot->Row = i / m_MaxUnitImageCount.X;

			icon->ColorAndOpacity = FLinearColor((float)(gridSlot->Column / m_MaxUnitImageCount.X), 0.0f, 0.0f, 1.0f);
			
			/*gridSlot->Padding = FMargin(4.0f);
			float unitHealthNorm = selectedUnit->m_UnitCoreComponent->CurrentHealth / selectedUnit->m_UnitCoreComponent->BaseHealth;
			if (unitHealthNorm > 0.0f)
			{
				icon->ColorAndOpacity = FMath::Lerp(UnitHealthColor_LowHealth, UnitHealthColor_FullHealth, unitHealthNorm);
			}
			else
			{
				icon->ColorAndOpacity = UnitHealthColor_Dead;
			}*/

		}
	}
	else if (newSelectedUnitIconCount == 1)
	{

	}
	else // No units are selected
	{
		// Pass empty array to signal no units are currently selected
		//TArray<FSelectedUnitIconInfo> selectedUnitInfos = {};
		//UpdateSelectedUnitIcons(selectedUnitInfos);
	}
}
