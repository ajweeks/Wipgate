// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_HUDBase.h"

#include "Components/Button.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/PanelSlot.h"
#include "Components/Image.h"
#include "Engine/Engine.h"

#include "RTS_Entity.h"
#include "RTS_Unit.h"
#include "RTS_Specialist.h"
#include "RTS_GameState.h"
#include "RTS_UnitIcon.h"

DEFINE_LOG_CATEGORY(RTS_HUD_BASE_LOG);

void URTS_HUDBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::Tick(MyGeometry, InDeltaTime);

	if (!SelectedEntityIconGridRef)
	{
		UE_LOG(RTS_HUD_BASE_LOG, Error, TEXT("Selected unit icons grid not set! (Should be set in HUD BP's constructor)"));
		return;
	}

	if (!CommandCardGridRef)
	{
		UE_LOG(RTS_HUD_BASE_LOG, Error, TEXT("Command card grid not set! (Should be set in HUD BP's constructor)"));
		return;
	}

	if (SelectedEntitiesRef.Num() > 1) // If one unit was selected, there won't be an icon for it
	{
		FIntPoint viewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
		FVector2D selectedUnitsBGImageAbsoluteSize = SelectedEntityIconGridRef->GetCachedGeometry().GetAbsoluteSize();
		selectedUnitsBGImageAbsoluteSize.X = FMath::Clamp(selectedUnitsBGImageAbsoluteSize.X, 0.0f, (float)viewportSize.X);
		selectedUnitsBGImageAbsoluteSize.Y = FMath::Clamp(selectedUnitsBGImageAbsoluteSize.Y, 0.0f, (float)viewportSize.Y);

		FVector2D iconSize = { 32, 32 };// SelectedUnitsRef[0]->Icon->GetCachedGeometry().GetAbsoluteSize();
		iconSize.X += EntityIconPadding.Left + EntityIconPadding.Right;
		iconSize.Y += EntityIconPadding.Top + EntityIconPadding.Bottom;

		FVector2D maxSelectedUnitImageCountF = (selectedUnitsBGImageAbsoluteSize / iconSize);
		FIntPoint maxSelectedUnitImageCount = FIntPoint(
			FMath::TruncToInt(maxSelectedUnitImageCountF.X) - 1, 
			FMath::TruncToInt(maxSelectedUnitImageCountF.Y) - 1);

		m_MaxEntityImageCount = maxSelectedUnitImageCount;
		UpdateSelectedEntities(SelectedEntitiesRef, false);
	}
}

void URTS_HUDBase::UpdateSelectedEntities(const TArray<ARTS_Entity*>& SelectedEntities, bool ClearArray)
{
	int32 newSelectedEntityCount = SelectedEntities.Num();
	int32 newSelectedEntityIconCount = FMath::Min(newSelectedEntityCount, m_MaxEntityImageCount.X * m_MaxEntityImageCount.Y);

	if (ClearArray)
	{
		ClearEntityIconsFromGrid();
		//if (SelectedEntitiesRef.Num() > 1)
		//{
		//	for (auto oldSelectedEntity : SelectedEntitiesRef)
		//	{
		//		RemoveEntityIconFromGrid(oldSelectedEntity->IconIndex);
		//		oldSelectedEntity->Icon = nullptr;
		//	}
		//}

		if (SelectedEntities.Num() > 1)
		{
			for (auto newSelectedEntity : SelectedEntities)
			{
				newSelectedEntity->Icon = WidgetTree->ConstructWidget<UImage>();
				AddEntityIconToGrid(newSelectedEntity);
			}
		}

		SelectedEntitiesRef = SelectedEntities;
	}

	if (newSelectedEntityIconCount > 1)
	{
		for (int32 i = 0; i < newSelectedEntityIconCount; ++i)
		{
			ARTS_Entity* entity = SelectedEntitiesRef[i];
			checkSlow(entity);

			int col = i % m_MaxEntityImageCount.X;
			int row = i / m_MaxEntityImageCount.X;
			float unitHealthNorm = (float)entity->CurrentDefenceStats.Health / (float)entity->BaseDefenceStats.Health;
			FLinearColor color;
			if (unitHealthNorm > 0.0f)
			{
				color = FMath::Lerp(EntityIconHealthColor_LowHealth, EntityIconHealthColor_FullHealth, unitHealthNorm);
			}
			else
			{
				color = EntityIconHealthColor_Dead;
			}

			UpdateEntityIconProperties(entity->IconIndex, col, row, color);
		}
	}
}

