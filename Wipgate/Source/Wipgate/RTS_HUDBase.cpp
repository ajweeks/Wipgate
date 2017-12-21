// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_HUDBase.h"

#include "Components/Button.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/PanelSlot.h"
#include "Components/Image.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

#include "RTS_Entity.h"
#include "RTS_Unit.h"
#include "RTS_Specialist.h"
#include "RTS_GameState.h"
#include "RTS_UnitIcon.h"
#include "RTS_PlayerController.h"

DEFINE_LOG_CATEGORY(RTS_HUD_BASE_LOG);

URTS_HUDBase::URTS_HUDBase(const FObjectInitializer& ObjectInitializer) :
	UUserWidget(ObjectInitializer)
{
	m_MaxEntityImageCount = { 13, 1 };
}

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

	//if (SelectedEntitiesRef.Num() > 1) // If one unit was selected, there won't be an icon for it
	//{
		//FIntPoint viewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
		//FVector2D selectedUnitsBGImageAbsoluteSize = SelectedEntityIconGridRef->GetCachedGeometry().GetAbsoluteSize();
		//selectedUnitsBGImageAbsoluteSize.X = FMath::Clamp(selectedUnitsBGImageAbsoluteSize.X, 0.0f, (float)viewportSize.X);
		//selectedUnitsBGImageAbsoluteSize.Y = FMath::Clamp(selectedUnitsBGImageAbsoluteSize.Y, 0.0f, (float)viewportSize.Y);

		//FVector2D iconSize = (FVector2D(32, 32));
		//iconSize.X += EntityIconPadding.Left + EntityIconPadding.Right;
		//iconSize.Y += EntityIconPadding.Top + EntityIconPadding.Bottom;

		//FVector2D maxSelectedUnitImageCountF = (selectedUnitsBGImageAbsoluteSize / iconSize);
		//FIntPoint maxSelectedUnitImageCount = FIntPoint(
		//	FMath::TruncToInt(maxSelectedUnitImageCountF.X) - 1, 
		//	FMath::TruncToInt(maxSelectedUnitImageCountF.Y) - 1);

		//m_MaxEntityImageCount = maxSelectedUnitImageCount;
		//UpdateSelectedEntities(SelectedEntitiesRef, false);
	//}


	/* Minimap frustum calculation
	UWorld* World = GetWorld();
	APlayerController* pc = World->GetFirstPlayerController();
	ARTS_PlayerController* rtsPlayerController = Cast<ARTS_PlayerController>(pc);
	if (rtsPlayerController)
	{
		FVector tlStartPos, trStartPos, blStartPos, brStartPos;
		FVector tlWorldDir, trWorldDir, blWorldDir, brWorldDir;
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		rtsPlayerController->DeprojectScreenPositionToWorld(0.0f, 0.0f, tlStartPos, tlWorldDir);
		rtsPlayerController->DeprojectScreenPositionToWorld(ViewportSize.X, 0.0f, trStartPos, trWorldDir);
		rtsPlayerController->DeprojectScreenPositionToWorld(0.0f, ViewportSize.Y, blStartPos, blWorldDir);
		rtsPlayerController->DeprojectScreenPositionToWorld(ViewportSize.X, ViewportSize.Y, brStartPos, brWorldDir);

		UE_LOG(RTS_HUD_BASE_LOG, Log, TEXT("Points: (%f, %f, %f), (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)"), 
			tlStartPos.X, tlStartPos.Y, tlStartPos.Z, trStartPos.X, trStartPos.Y, trStartPos.Z, 
			blStartPos.X, blStartPos.Y, blStartPos.Z, brStartPos.X, brStartPos.Y, brStartPos.Z);

		float dist = 1000000.0f;
		FVector tlEndPos = tlStartPos + tlWorldDir * dist;
		FVector trEndPos = trStartPos + trWorldDir * dist;
		FVector blEndPos = blStartPos + blWorldDir * dist;
		FVector brEndPos = brStartPos + brWorldDir * dist;

		//DrawDebugPoint(World, tlTracePos, 10.0f, FColor::White, true);
		//DrawDebugPoint(World, trTracePos, 10.0f, FColor::Red, true);
		//DrawDebugPoint(World, blTracePos, 10.0f, FColor::Green, true);
		//DrawDebugPoint(World, brTracePos, 10.0f, FColor::Blue, true);

		FHitResult hitResultTL, hitResultTR, hitResultBL, hitResultBR;
		ECollisionChannel collisionChannel = ECollisionChannel::ECC_Camera;
		World->LineTraceSingleByChannel(hitResultTL, tlWorldDir, tlEndPos, collisionChannel);
		World->LineTraceSingleByChannel(hitResultTR, trWorldDir, trEndPos, collisionChannel);
		World->LineTraceSingleByChannel(hitResultBL, blWorldDir, blEndPos, collisionChannel);
		World->LineTraceSingleByChannel(hitResultBR, brWorldDir, brEndPos, collisionChannel);

		FVector tlHitLocation = hitResultTL.Location + FVector(0, 0, 20);
		FVector trHitLocation = hitResultTR.Location + FVector(0, 0, 20);
		FVector blHitLocation = hitResultBL.Location + FVector(0, 0, 20);
		FVector brHitLocation = hitResultBR.Location + FVector(0, 0, 20);

		DrawDebugLine(World, tlHitLocation, tlHitLocation, FColor::White, true);
		DrawDebugLine(World, trHitLocation, trHitLocation, FColor::White, true);
		DrawDebugLine(World, blHitLocation, blHitLocation, FColor::White, true);
		DrawDebugLine(World, brHitLocation, brHitLocation, FColor::White, true);

		//DrawDebugLine(World, tlStartPos, tlHitLocation, FColor::White, true);
		//DrawDebugLine(World, trStartPos, trHitLocation, FColor::Red, true);
		//DrawDebugLine(World, blStartPos, brHitLocation, FColor::Green, true);
		//DrawDebugLine(World, brStartPos, blHitLocation, FColor::Blue, true);

		//DrawDebugSphere(World, tlHitLocation, 10.0f, 12, FColor::White, true);
		//DrawDebugSphere(World, trHitLocation, 10.0f, 12, FColor::Red, true);
		//DrawDebugSphere(World, blHitLocation, 10.0f, 12, FColor::Green, true);
		//DrawDebugSphere(World, brHitLocation, 10.0f, 12, FColor::Blue, true);

		//hitResultTL.ImpactPoint
	}
	*/
}

void URTS_HUDBase::UpdateSelectedEntities(const TArray<ARTS_Entity*>& SelectedEntities, bool ClearArray)
{
	int32 newSelectedEntityCount = SelectedEntities.Num();
	int32 newSelectedEntityIconCount = FMath::Min(newSelectedEntityCount, m_MaxEntityImageCount.X * m_MaxEntityImageCount.Y);

	if (ClearArray)
	{
		ClearEntityIconsFromGrid();

		if (SelectedEntities.Num() > 1)
		{
			int32 createdIconCount = 0;
			for (auto newSelectedEntity : SelectedEntities)
			{
				newSelectedEntity->Icon = WidgetTree->ConstructWidget<UImage>();
				AddEntityIconToGrid(newSelectedEntity);
				if (++createdIconCount >= newSelectedEntityIconCount)
				{
					break;
				}
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

void URTS_HUDBase::DeselectEntity(ARTS_Entity* Entity)
{
	ClearEntityIconsFromGrid();

	AGameStateBase* baseGameState = GetWorld()->GetGameState();
	ARTS_GameState* RTS_GameState = Cast<ARTS_GameState>(baseGameState);

	RTS_GameState->SelectedEntities.Remove(Entity);
	UpdateSelectedEntities(RTS_GameState->SelectedEntities, true);
}
