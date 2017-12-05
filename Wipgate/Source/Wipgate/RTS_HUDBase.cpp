// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_HUDBase.h"

#include "Components/Button.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/PanelSlot.h"
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

	if (!SelectedUnitIconGridRef)
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
		FVector2D selectedUnitsBGImageAbsoluteSize = SelectedUnitIconGridRef->GetCachedGeometry().GetAbsoluteSize();
		selectedUnitsBGImageAbsoluteSize.X = FMath::Clamp(selectedUnitsBGImageAbsoluteSize.X, 0.0f, (float)viewportSize.X);
		selectedUnitsBGImageAbsoluteSize.Y = FMath::Clamp(selectedUnitsBGImageAbsoluteSize.Y, 0.0f, (float)viewportSize.Y);

		FVector2D iconSize = { 32, 32 };// SelectedUnitsRef[0]->Icon->GetCachedGeometry().GetAbsoluteSize();
		iconSize.X += UnitIconPadding.Left + UnitIconPadding.Right;
		iconSize.Y += UnitIconPadding.Top + UnitIconPadding.Bottom;

		FVector2D maxSelectedUnitImageCountF = (selectedUnitsBGImageAbsoluteSize / iconSize);
		FIntPoint maxSelectedUnitImageCount = FIntPoint(
			FMath::TruncToInt(maxSelectedUnitImageCountF.X) - 1, 
			FMath::TruncToInt(maxSelectedUnitImageCountF.Y) - 1);

		m_MaxUnitImageCount = maxSelectedUnitImageCount;
		UpdateSelectedUnits(SelectedEntitiesRef, false);
	}
}

void URTS_HUDBase::UpdateSelectedUnits(const TArray<ARTS_Entity*>& SelectedEntities, bool ClearArray)
{
	int32 newSelectedUnitCount = SelectedEntities.Num();
	int32 newSelectedUnitIconCount = FMath::Min(newSelectedUnitCount, m_MaxUnitImageCount.X * m_MaxUnitImageCount.Y);

	if (ClearArray)
	{
		if (SelectedEntitiesRef.Num() > 1)
		{
			for (auto oldSelectedUnit : SelectedEntitiesRef)
			{
				RemoveUnitIconFromGrid(oldSelectedUnit->Icon);
				oldSelectedUnit->Icon = nullptr;
			}
		}

		if (SelectedEntities.Num() > 1)
		{
			for (auto newSelectedUnit : SelectedEntities)
			{
				newSelectedUnit->Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
				AddUnitIconToGrid(newSelectedUnit->Icon);
			}
		}

		SelectedEntitiesRef = SelectedEntities;
	}

	if (newSelectedUnitIconCount > 1)
	{
		for (int32 i = 0; i < newSelectedUnitIconCount; ++i)
		{
			ARTS_Entity* entity = SelectedEntitiesRef[i];
			checkSlow(entity);

			int col = i % m_MaxUnitImageCount.X;
			int row = i / m_MaxUnitImageCount.X;
			float unitHealthNorm = (float)entity->CurrentDefenceStats.Health/ (float)entity->BaseDefenceStats.Health;
			FLinearColor color;
			if (unitHealthNorm > 0.0f)
			{
				color = FMath::Lerp(UnitHealthColor_LowHealth, UnitHealthColor_FullHealth, unitHealthNorm);
			}
			else
			{
				color = UnitHealthColor_Dead;
			}
			UE_LOG(RTS_HUD_BASE_LOG, Log, TEXT("color: %f, %f, %f"), color.R, color.G, color.B);

			UpdateUnitIconProperties(entity->Icon, col, row, color);
		}
	}
}

void URTS_HUDBase::OnUnitIconPressed(ARTS_UnitCharacter* unit)
{
	AGameStateBase* baseGameState = GetWorld()->GetGameState();
	auto gameState = Cast<ARTS_GameState>(baseGameState);
}
