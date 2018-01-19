// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_PlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/Button.h"
#include "Components/BoxComponent.h"
#include "Components/ProgressBar.h"
#include "Components/StaticMeshComponent.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "Engine/UserInterfaceSettings.h"
#include "Engine/RendererSettings.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "AI/Navigation/NavigationSystem.h"

#include "Ability.h"
#include "AbilityIconBase.h"
#include "GeneralFunctionLibrary_CPP.h"
#include "RTS_Cursor.h"
#include "RTS_Entity.h"
#include "RTS_GameState.h"
#include "RTS_GameInstance.h"
#include "RTS_HUDBase.h"
#include "RTS_LevelEnd.h"
#include "RTS_LevelBounds.h"
#include "RTS_PlayerSpawner.h"
#include "RTS_Specialist.h"
#include "RTS_Team.h"
#include "RTS_Unit.h"
#include "RTS_AIController.h"
#include "WipgateGameModeBase.h"
#include "UpgradeShopBase.h"

DEFINE_LOG_CATEGORY_STATIC(RTS_PlayerController_Log, Log, All);

ARTS_PlayerController::ARTS_PlayerController()
{
	//AttemptToFindObjectByPath(&MainHUDInstance, TEXT("WidgetBlueprint'/Game/Code/User_Interface/RTS_HUD_BP.RTS_HUD_BP'"));

	//AttemptToFindObjectByPath(&AbilityMovementMove, TEXT("BlueprintGeneratedClass'/Game/Code/Ablities/Movement/Ab_Movement_Move.Ab_Movement_Move_C'"));
	//AttemptToFindObjectByPath(&AbilityMovementAttackMove, TEXT("BlueprintGeneratedClass'/Game/Code/Ablities/Movement/Ab_Movement_AttackMove.Ab_Movement_AttackMove_C'"));
	//AttemptToFindObjectByPath(&AbilityMovementStop, TEXT("BlueprintGeneratedClass'/Game/Code/Ablities/Movement/Ab_Movement_Stop.Ab_Movement_Stop_C'"));
	//AttemptToFindObjectByPath(&AbilityMovementHoldPosition, TEXT("BlueprintGeneratedClass'/Game/Code/Ablities/Movement/Ab_Movement_HoldPosition.Ab_Movement_HoldPosition_C'"));
	//AttemptToFindObjectByPath(&AbilityLumaApply, TEXT("BlueprintGeneratedClass'/Game/Code/Ablities/Luma/Ab_Luma_Apply.Ab_Luma_Apply_C'"));

	bShowMouseCursor = true;
}

void ARTS_PlayerController::Initialize()
{
	// Get references to camera and its components
	m_RTS_CameraPawn = GetPawn();
	check(m_RTS_CameraPawn != nullptr);
	
	auto baseGameMode = GetWorld()->GetAuthGameMode();
	AWipgateGameModeBase* castedGameMode = Cast<AWipgateGameModeBase>(baseGameMode);
	m_LevelStartLocation = FVector::ZeroVector;
	if (castedGameMode)
	{
		ARTS_PlayerSpawner* playerSpawner = castedGameMode->GetPlayerSpawner();
		if (playerSpawner)
		{
			m_LevelStartLocation = playerSpawner->GetActorLocation();
		}

		m_LevelBounds = castedGameMode->GetLevelBounds();
		if (!m_LevelBounds)
		{
			PrintStringToScreen("Level bounds not set!", FColor::Red, 10.0f);
		}

		ARTS_LevelEnd* levelEnd = castedGameMode->GetLevelEnd();
		if (levelEnd)
		{
			m_LevelEndLocation = levelEnd->GetActorLocation();
		}	
	}

	m_RTS_CameraPawn->SetActorLocation(m_LevelStartLocation);
	m_RTS_CameraPawn->SetActorRotation(m_StartingRotation);

	// Move to level end after delay (to let world load in)
	if (m_MoveToLevelEndAtStartup)
	{
		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARTS_PlayerController::StartMovingToLevelEnd, m_DelayBeforeMovingToLevelEnd, false);
	}

	TArray<UCameraComponent*> cameraComponents;
	m_RTS_CameraPawn->GetComponents(cameraComponents);
	if (cameraComponents.Num() > 0)
	{
		m_RTS_CameraPawnCameraComponent = cameraComponents[0];
	}
	else
	{
		UE_LOG(RTS_PlayerController_Log, Error, TEXT("Camera pawn doesn't contain a camera component!"));
	}
	check(m_RTS_CameraPawnCameraComponent != nullptr);

	TArray<UStaticMeshComponent*> meshComponents;
	m_RTS_CameraPawn->GetComponents(meshComponents);
	if (meshComponents.Num() > 0)
	{
		m_RTS_CameraPawnMeshComponent = meshComponents[0];
	}
	else
	{
		UE_LOG(RTS_PlayerController_Log, Error, TEXT("Camera pawn doesn't contain a mesh component!"));
	}
	check(m_RTS_CameraPawnMeshComponent != nullptr);

	TArray<USpringArmComponent*> springArmComponents;
	m_RTS_CameraPawn->GetComponents(springArmComponents);
	if (springArmComponents.Num() > 0)
	{
		m_RTS_CameraPawnSpringArmComponent = springArmComponents[0];
	}
	else
	{
		UE_LOG(RTS_PlayerController_Log, Error, TEXT("Camera pawn doesn't contain a spring arm component!"));
	}
	check(m_RTS_CameraPawnSpringArmComponent != nullptr);

	// Create and add HUD to viewport
	if (MainHUD)
	{
		MainHUDInstance = CreateWidget<UUserWidget>(this, MainHUD);

		if (MainHUDInstance)
		{
			MainHUDInstance->AddToViewport();
			m_RTSHUD = Cast<URTS_HUDBase>(MainHUDInstance);
			m_RTSHUD->PlayerController = this;
		}
		else
		{
			UE_LOG(RTS_PlayerController_Log, Error, TEXT("Failed to create main HUD widget!"));
		}
	}
	else
	{
		UE_LOG(RTS_PlayerController_Log, Error, TEXT("Main HUD template was not set in player controller BP!"));
	}

	AGameStateBase* baseGameState = GetWorld()->GetGameState();
	m_RTS_GameState = Cast<ARTS_GameState>(baseGameState);
	check(m_RTS_GameState != nullptr);

	// Set input mode to show cursor when captured (clicked) and to lock cursor to viewport
	FInputModeGameAndUI inputMode;
	inputMode.SetHideCursorDuringCapture(false);
	inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(inputMode);

	if (m_RTSHUD)
	{
		m_RTSHUD->AddSelectionGroupIconsToGrid(SELECTION_GROUP_COUNT);
	}

	auto gameinstance = Cast<URTS_GameInstance>(GetGameInstance());
	if (gameinstance)
	{
		if (gameinstance->CurrentRound == 0)
		{
			AddLuma(m_StartingLumaAmount);
		}
	}
}

void ARTS_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Primary Click", IE_Pressed, this, &ARTS_PlayerController::ActionPrimaryClickPressed);
	InputComponent->BindAction("Primary Click", IE_Released, this, &ARTS_PlayerController::ActionPrimaryClickReleased);
	InputComponent->BindAction("Secondary Click", IE_Pressed, this, &ARTS_PlayerController::ActionSecondaryClickPressed);
	InputComponent->BindAction("Secondary Click", IE_Released, this, &ARTS_PlayerController::ActionSecondaryClickReleased);
	InputComponent->BindAction("Tertiary Click", IE_Pressed, this, &ARTS_PlayerController::ActionTertiaryClickPressed);
	InputComponent->BindAction("Tertiary Click", IE_Released, this, &ARTS_PlayerController::ActionTertiaryClickReleased);
	InputComponent->BindAction("Move Fast", IE_Pressed, this, &ARTS_PlayerController::ActionMoveFastPressed);
	InputComponent->BindAction("Move Fast", IE_Released, this, &ARTS_PlayerController::ActionMoveFastReleased);

	InputComponent->BindAction("Ability Specialist Active Select", IE_Pressed, this, &ARTS_PlayerController::OnAbilitySpecialistActiveSelect);
	InputComponent->BindAction("Ability Specialist Construct Select", IE_Pressed, this, &ARTS_PlayerController::OnAbilitySpecialistConstructSelect);
	InputComponent->BindAction("Ability Specialist Passive Select", IE_Pressed, this, &ARTS_PlayerController::OnAbilitySpecialistPassiveSelect);

	InputComponent->BindAction("Ability Movement Move Select", IE_Pressed, this, &ARTS_PlayerController::OnAbilityMovementMoveSelect);
	InputComponent->BindAction("Ability Movement Attack Move Select", IE_Pressed, this, &ARTS_PlayerController::OnAbilityMovementAttackMoveSelect);
	InputComponent->BindAction("Ability Movement Stop Select", IE_Pressed, this, &ARTS_PlayerController::OnAbilityMovementStopSelect);
	InputComponent->BindAction("Ability Movement Hold Position Select", IE_Pressed, this, &ARTS_PlayerController::OnAbilityMovementHoldPositionSelect);

	InputComponent->BindAction("Ability Luma Apply Select", IE_Pressed, this, &ARTS_PlayerController::OnAbilityLumaApplySelect);

	InputComponent->BindAction("Selection Group 1", IE_Pressed, this, &ARTS_PlayerController::ActionSelectionGroup1);
	InputComponent->BindAction("Create Selection Group 1", IE_Pressed, this, &ARTS_PlayerController::ActionCreateSelectionGroup1);
	InputComponent->BindAction("Selection Group 2", IE_Pressed, this, &ARTS_PlayerController::ActionSelectionGroup2);
	InputComponent->BindAction("Create Selection Group 2", IE_Pressed, this, &ARTS_PlayerController::ActionCreateSelectionGroup2);
	InputComponent->BindAction("Selection Group 3", IE_Pressed, this, &ARTS_PlayerController::ActionSelectionGroup3);
	InputComponent->BindAction("Create Selection Group 3", IE_Pressed, this, &ARTS_PlayerController::ActionCreateSelectionGroup3);
	InputComponent->BindAction("Selection Group 4", IE_Pressed, this, &ARTS_PlayerController::ActionSelectionGroup4);
	InputComponent->BindAction("Create Selection Group 4", IE_Pressed, this, &ARTS_PlayerController::ActionCreateSelectionGroup4);
	InputComponent->BindAction("Selection Group 5", IE_Pressed, this, &ARTS_PlayerController::ActionSelectionGroup5);
	InputComponent->BindAction("Create Selection Group 5", IE_Pressed, this, &ARTS_PlayerController::ActionCreateSelectionGroup5);

	InputComponent->BindAction("Center On Selection", IE_Pressed, this, &ARTS_PlayerController::ActionCenterOnSelectionPressed);
	InputComponent->BindAction("Center On Selection", IE_Released, this, &ARTS_PlayerController::ActionCenterOnSelectionReleased);

	InputComponent->BindAction("Add To Selection", IE_Pressed, this, &ARTS_PlayerController::ActionAddToSelectionPressed);
	InputComponent->BindAction("Add To Selection", IE_Released, this, &ARTS_PlayerController::ActionAddToSelectionReleased);

	InputComponent->BindAction("Invert Selection", IE_Pressed, this, &ARTS_PlayerController::InvertSelection);

	InputComponent->BindAxis("Zoom", this, &ARTS_PlayerController::AxisZoom);
}

bool ARTS_PlayerController::IsEdgeMovementEnabled() const
{
	return m_EdgeMovementEnabled;
}

void ARTS_PlayerController::SetEdgeMovementEnabled(bool enabled)
{
	m_EdgeMovementEnabled = enabled;
}

void ARTS_PlayerController::UpdateSelectedEntitiesBase()
{
	for (int32 i = 0; i < m_RTS_GameState->SelectedEntities.Num(); /* */)
	{
		if (m_RTS_GameState->SelectedEntities[i]->IsSelectableByPlayer())
		{
			m_RTS_GameState->SelectedEntities[i]->SetSelected(true);
			++i;
		}
		else
		{
			m_RTS_GameState->SelectedEntities[i]->SetSelected(false);
			m_RTS_GameState->SelectedEntities.RemoveAt(i);
		}
	}

	UpdateSelectedEntities(m_RTS_GameState->SelectedEntities);
	m_RTSHUD->UpdateSelectedEntities(m_RTS_GameState->SelectedEntities);

	int32 currentNumEntitiesSelected = m_RTS_GameState->SelectedEntities.Num();
	if (currentNumEntitiesSelected == 0)
	{
		m_RTSHUD->HideLumaAbilityIcons();
		m_RTSHUD->HideMovementAbilityIcons();
	}
	else
	{
		m_RTSHUD->ShowLumaAbilityIcons();
		m_RTSHUD->ShowMovementAbilityIcons();
	}

	if (currentNumEntitiesSelected == 1)
	{
		ARTS_Entity* entity = m_RTS_GameState->SelectedEntities[0];
		ARTS_Specialist* specialist = Cast<ARTS_Specialist>(entity);
		if (specialist && specialist->IsSelectableByPlayer())
		{
			m_SpecialistShowingAbilities = specialist;
			CreateSpecialistAbilityButtons();
		}

		if (entity->IsSelectableByPlayer())
		{
			m_RTSHUD->ShowSelectedEntityStats(entity);
		}
	}
	else
	{
		m_RTSHUD->HideSelectedEntityStats();
	}
}

void ARTS_PlayerController::Tick(float DeltaSeconds)
{
	if (!m_RTS_GameState ||!m_RTSHUD)
	{
		return;
	}

	if (IsPaused())
	{
		// Don't show selection box while paused
		m_ClickStartSS = FVector2D::ZeroVector;
		m_ClickEndSS = FVector2D::ZeroVector;

		m_RTSHUD->UpdateSelectionBox(FVector2D::ZeroVector, FVector2D::ZeroVector);

		return;
	}
	
	
	if (!SelectedAbility && IsInputKeyDown(EKeys::LeftMouseButton))
	{
		// Update selection box size if mouse is being dragged
		m_ClickEndSS = UGeneralFunctionLibrary_CPP::GetMousePositionVector2D(this);

		FVector2D selectionBoxPosition = m_ClickStartSS;
		FVector2D selectionBoxSize = (m_ClickEndSS - m_ClickStartSS);

		m_RTSHUD->UpdateSelectionBox(selectionBoxPosition, selectionBoxSize);
	}


	if (m_ZoomingToTarget)
	{
		float armDeltaLength = (m_TargetZoomArmLength - m_RTS_CameraPawnSpringArmComponent->TargetArmLength) * DeltaSeconds * m_ZoomSpeed;
		m_RTS_CameraPawnSpringArmComponent->TargetArmLength += armDeltaLength;

		if ((armDeltaLength > 0.0f &&
			m_RTS_CameraPawnSpringArmComponent->TargetArmLength >= m_TargetZoomArmLength) ||
			(armDeltaLength < 0.0f &&
				m_RTS_CameraPawnSpringArmComponent->TargetArmLength <= m_TargetZoomArmLength))
		{
			m_RTS_CameraPawnSpringArmComponent->TargetArmLength = m_TargetZoomArmLength;
			m_ZoomingToTarget = false;
		}
	}

	const float realTimeSeconds = GetWorld()->GetRealTimeSeconds();

	if (m_MovingToTarget)
	{
		MoveToTarget();
		return;
	}
	else
	{
		if (realTimeSeconds > m_EdgeModeDisableDelaySec &&
			!m_MoveToLevelEndAtStartup || (m_MoveToLevelEndAtStartup && m_ReturnedToStartAfterViewingEnd) &&
			m_RTS_CameraPawnMeshComponent && m_RTS_CameraPawn)
		{
			FVector rightVec = m_RTS_CameraPawnMeshComponent->GetRightVector();
			FVector forwardVec = m_RTS_CameraPawnMeshComponent->GetForwardVector();
			forwardVec.Z = 0; // Only move along XY plane
			forwardVec.Normalize();

			float camDistSpeedMultiplier = 0.0f;
			UWorld* world = GetWorld();
			if (world)
			{
				camDistSpeedMultiplier = CalculateMovementSpeedBasedOnCameraZoom();
			}
			else
			{
				UE_LOG(RTS_PlayerController_Log, Error, TEXT("World not found!"));
			}
			FVector pCamLocation = m_RTS_CameraPawn->GetActorLocation();

			if (m_Panning)
			{
				FVector2D currentMousePosNorm = UGeneralFunctionLibrary_CPP::GetNormalizedMousePosition(this);
				FVector2D dMousePos = currentMousePosNorm - m_PanMouseStartLocationSSNorm;

				FVector targetDCamLocation = FVector::ZeroVector;
				targetDCamLocation = -dMousePos.X * rightVec * m_PanSpeed * m_FastMoveMultiplier * camDistSpeedMultiplier;
				targetDCamLocation += dMousePos.Y * forwardVec * m_PanSpeed * m_FastMoveMultiplier * camDistSpeedMultiplier;

				FVector targetCamLocation = m_PanCamStartLocation + targetDCamLocation;

				targetCamLocation = ClampCamPosWithBounds(targetCamLocation);

				m_RTS_CameraPawn->SetActorLocation(targetCamLocation);
			}
			else if (m_EdgeMovementEnabled)
			{
				FVector targetDCamLocation = FVector::ZeroVector;

				FVector2D normMousePos = UGeneralFunctionLibrary_CPP::GetNormalizedMousePosition(this);
				if (normMousePos.X > 1.0f - m_EdgeSize)
				{
					targetDCamLocation = rightVec * m_EdgeMoveSpeed * m_FastMoveMultiplier * camDistSpeedMultiplier * DeltaSeconds;
					m_MovingToSelectionCenter = false;
				}
				else if (normMousePos.X < m_EdgeSize)
				{
					targetDCamLocation = -rightVec * m_EdgeMoveSpeed * m_FastMoveMultiplier * camDistSpeedMultiplier * DeltaSeconds;
					m_MovingToSelectionCenter = false;
				}

				if (normMousePos.Y > 1.0f - m_EdgeSize)
				{
					targetDCamLocation -= forwardVec * m_EdgeMoveSpeed * m_FastMoveMultiplier * camDistSpeedMultiplier * DeltaSeconds;
					m_MovingToSelectionCenter = false;
				}
				else if (normMousePos.Y < m_EdgeSize)
				{
					targetDCamLocation += forwardVec * m_EdgeMoveSpeed * m_FastMoveMultiplier * camDistSpeedMultiplier * DeltaSeconds;
					m_MovingToSelectionCenter = false;
				}

				targetDCamLocation = ClampDCamPosWithBounds(targetDCamLocation);

				m_RTS_CameraPawn->AddActorWorldOffset(targetDCamLocation);
			}

			if (m_MovingToSelectionCenter || AlwaysCenterOnUnits)
			{
				MoveToCenterOfUnits(!AlwaysCenterOnUnits);
			}
		}
	}

	static const auto mainClickKeys = PlayerInput->GetKeysForAction("Primary Click");
	static const FKey mainClickButton = mainClickKeys[0].Key;
	const bool isPrimaryClickButtonDown = IsInputKeyDown(mainClickButton);
	const bool isPrimaryClickButtonClicked = WasInputKeyJustPressed(mainClickButton);
	const bool isPrimaryClickButtonReleased = WasInputKeyJustReleased(mainClickButton);

	static const auto addToSelectionKeys = PlayerInput->GetKeysForAction("Add To Selection");
	static const FKey addToSelectionKey = addToSelectionKeys[0].Key;
	const bool isAddToSelectionKeyDown = IsInputKeyDown(addToSelectionKey);
	const bool isAddToSelectionKeyPressed = WasInputKeyJustPressed(addToSelectionKey);
	const bool isAddToSelectionKeyReleased = WasInputKeyJustReleased(addToSelectionKey);


	if (isPrimaryClickButtonDown)
	{
		if (IsCursorOverPurchasableItem())
		{
			// Player clicked on a powerup, don't deselect anything
			CursorRef->SetCursorTexture(CursorRef->DefaultTexture);
			return;
		}
	}

	if (!isAddToSelectionKeyDown)
	{
		for (int i = 0; i < m_RTS_GameState->Entities.Num(); ++i)
		{
			ARTS_Entity* entity = m_RTS_GameState->Entities[i];
			if (!m_RTS_GameState->SelectedEntities.Contains(entity))
			{
				entity->SetSelected(false);
			}
		}
	}

	static ETraceTypeQuery pawnTraceType = UEngineTypes::ConvertToTraceType(ECC_Pawn);
	FHitResult pawnHitResult;
	GetHitResultUnderCursorByChannel(pawnTraceType, false, pawnHitResult);
	AActor* actorUnderCursor = pawnHitResult.Actor.Get();
	static ARTS_Entity* entityUnderCursor = nullptr;
	ARTS_Entity* prevEntityUnderCursor = entityUnderCursor;
	entityUnderCursor = nullptr;

	FVector2D selectionBoxMin = m_ClickStartSS;
	FVector2D selectionBoxMax = m_ClickEndSS;
	UGeneralFunctionLibrary_CPP::FVector2DMinMax(selectionBoxMin, selectionBoxMax);

	FVector2D viewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	const UUserInterfaceSettings* uiSettings = GetDefault<UUserInterfaceSettings>();
	float viewportScale = uiSettings->GetDPIScaleBasedOnSize(FIntPoint((int)viewportSize.X, (int)viewportSize.Y));

	const int32 prevSelectedEntityCount = m_RTS_GameState->SelectedEntities.Num();

	for (auto entity : m_RTS_GameState->Entities)
	{
		FVector entityLocation = entity->GetActorLocation();

		// Draw entity bounding box
		if (entity->ShowSelectionBox)
		{
			UKismetSystemLibrary::DrawDebugBox(GetWorld(), entityLocation, entity->SelectionHitBox, FColor::White, FRotator::ZeroRotator, 2.0f, 4.0f);
		}

		if (entity->SelectionHitBox == FVector::ZeroVector)
		{
			UE_LOG(RTS_PlayerController_Log, Error, TEXT("Unit's selection hit box is (0, 0, 0)!"));
		}

		bool entityInSelectionBox = false;
		if (isPrimaryClickButtonDown)
		{
			// Check if this entity's min or max bounding box points lie within the selection box
			/*
			    
			     ^
			    / \
			   /   \
			  <     >
			  |\   /|
			  | \ / |
			  |  v  |
			  |  |  |
			  |  |  |
			  |  |  |
			  \  |  /
			   \ | /
			     v
			
			*/
			FVector entityBounds0 = entityLocation + FVector(
				-entity->SelectionHitBox.X,
				0.0f,
				-entity->SelectionHitBox.Z);
			FVector entityBounds1 = entityLocation + FVector(
				0.0f,
				-entity->SelectionHitBox.Y,
				-entity->SelectionHitBox.Z);
			FVector entityBounds2 = entityLocation + FVector(
				entity->SelectionHitBox.X,
				0.0f,
				-entity->SelectionHitBox.Z);
			FVector entityBounds3 = entityLocation + FVector(
				0.0f,
				entity->SelectionHitBox.Y,
				-entity->SelectionHitBox.Z);
			FVector entityBounds4 = entityLocation + FVector(
				-entity->SelectionHitBox.X,
				0.0f,
				entity->SelectionHitBox.Z);
			FVector entityBounds5 = entityLocation + FVector(
				0.0f,
				-entity->SelectionHitBox.Y,
				entity->SelectionHitBox.Z);
			FVector entityBounds6 = entityLocation + FVector(
				entity->SelectionHitBox.X,
				0.0f,
				entity->SelectionHitBox.Z);
			FVector entityBounds7 = entityLocation + FVector(
				0.0f,
				entity->SelectionHitBox.Y,
				entity->SelectionHitBox.Z);

			TArray<FVector2D> entityBoundsSS;
			entityBoundsSS.AddUninitialized(8);

			ProjectWorldLocationToScreen(entityBounds0, entityBoundsSS[0], true);
			ProjectWorldLocationToScreen(entityBounds1, entityBoundsSS[1], true);
			ProjectWorldLocationToScreen(entityBounds2, entityBoundsSS[2], true);
			ProjectWorldLocationToScreen(entityBounds3, entityBoundsSS[3], true);
			ProjectWorldLocationToScreen(entityBounds4, entityBoundsSS[4], true);
			ProjectWorldLocationToScreen(entityBounds5, entityBoundsSS[5], true);
			ProjectWorldLocationToScreen(entityBounds6, entityBoundsSS[6], true);
			ProjectWorldLocationToScreen(entityBounds7, entityBoundsSS[7], true);

			TArray<float> entityBoundsSSX;
			TArray<float> entityBoundsSSY;
			TArray<float> entityBoundsSSZ;

			FVector2D entityBoundsMinSS = FVector2D(MAX_flt, MAX_flt);
			FVector2D entityBoundsMaxSS = FVector2D(MIN_flt, MIN_flt);

			for (int32 v = 0; v < 8; ++v)
			{
				entityBoundsMinSS.X = FMath::Min(entityBoundsSS[v].X, entityBoundsMinSS.X);
				entityBoundsMinSS.Y = FMath::Min(entityBoundsSS[v].Y, entityBoundsMinSS.Y);
				entityBoundsMaxSS.X = FMath::Max(entityBoundsSS[v].X, entityBoundsMaxSS.X);
				entityBoundsMaxSS.Y = FMath::Max(entityBoundsSS[v].Y, entityBoundsMaxSS.Y);
			}

			entityBoundsMinSS /= viewportScale;
			entityBoundsMaxSS /= viewportScale;

			entityInSelectionBox = UGeneralFunctionLibrary_CPP::BoxesOverlap(entityBoundsMinSS, entityBoundsMaxSS, selectionBoxMin, selectionBoxMax);
		}

		// Check if entity is under mouse cursor (for single clicks)
		bool isThisUnitUnderCursor = (actorUnderCursor == entity);

		if (isThisUnitUnderCursor)
		{
			entityUnderCursor = entity;
		}

		const bool entityIsSelectable = entity->IsSelectableByPlayer();

		const bool entityWasSelected = entity->IsSelected();
		bool entityDeselected = isThisUnitUnderCursor && isAddToSelectionKeyDown && entityWasSelected && isPrimaryClickButtonClicked;
		bool entityWasLikelyDeselectedLastFrame = isThisUnitUnderCursor && isAddToSelectionKeyDown && isPrimaryClickButtonDown && !isPrimaryClickButtonClicked && !entityWasSelected;

		if (!SelectedAbility && 
			(entity->Alignment == ETeamAlignment::E_PLAYER ||
				entity->Alignment == ETeamAlignment::E_NEUTRAL_AI))
		{
			if (!entityIsSelectable)
			{
				if (m_RTS_GameState->SelectedEntities.Contains(entity))
				{
					m_RTS_GameState->SelectedEntities.Remove(entity);
					UpdateSelectedEntitiesBase();
				}
			}
			else // Entity is alive and not overdosed
			{
				if (entityDeselected)
				{
					m_RTS_GameState->SelectedEntities.Remove(entity);
					entity->SetSelected(false);
				}
				else if (entityInSelectionBox || isThisUnitUnderCursor)
				{
					if (!entityWasLikelyDeselectedLastFrame)
					{
						entity->SetSelected(true);

						if ((isAddToSelectionKeyDown && !isPrimaryClickButtonReleased))
						{
							m_RTS_GameState->SelectedEntities.AddUnique(entity);

						}
					}
				}
				else
				{
					if (isPrimaryClickButtonDown && !isAddToSelectionKeyDown && m_RTS_GameState->SelectedEntities.Contains(entity))
					{
						entity->SetSelected(false);
					}
				}
			}
		}
	}

	if (m_SpecialistShowingAbilities)
	{
		if (m_SpecialistShowingAbilities->IsSelectableByPlayer())
		{
			UpdateSpecialistAbilityButtons();
		}
		else
		{
			ClearSpecialistAbilityButtons();
			m_RTS_GameState->SelectedEntities.Empty();
			m_RTS_GameState->Entities.Remove(m_SpecialistShowingAbilities);
			m_SpecialistShowingAbilities = nullptr;
		}
	}

	if (m_RTS_GameState->SelectedEntities.Num() == 1)
	{
		ARTS_Entity* selectedEntity = m_RTS_GameState->SelectedEntities[0];
		if (selectedEntity->Health <= 0)
		{
			selectedEntity = nullptr;
			m_RTS_GameState->SelectedEntities.Empty();
			UpdateSelectedEntitiesBase();
		}
		else if (selectedEntity->IsSelectableByPlayer())
		{
			m_RTSHUD->ShowSelectedEntityStats(selectedEntity);
		}
	}

	if (prevSelectedEntityCount == 1 && m_RTS_GameState->SelectedEntities.Num() != 1)
	{
		m_RTSHUD->HideSelectedEntityStats();
	}

	static bool shiftWasDown = IsInputKeyDown(FKey("LeftShift"));
	bool shiftIsDown = IsInputKeyDown(FKey("LeftShift"));
	if (shiftWasDown && !shiftIsDown)
	{
		if (m_AbilityOnShift)
		{
			if (m_AbilityOnShift->Icon)
			{
				m_AbilityOnShift->Icon->OnAbilityActivate();
			}
			m_AbilityOnShift->Deselect();
			m_AbilityOnShift = nullptr;
			SelectedAbility = nullptr;
		}
	}
	shiftWasDown = shiftIsDown;

	/* Cursor */
	if (CursorRef)
	{
		if (SelectedAbility)
		{
			switch (SelectedAbility->Type)
			{
			case EAbilityType::E_TARGET_ALLY:
			{
				if (entityUnderCursor &&
					(entityUnderCursor->Alignment == ETeamAlignment::E_PLAYER ||
						entityUnderCursor->Alignment == ETeamAlignment::E_NEUTRAL_AI))
				{
					CursorRef->SetCursorTexture(CursorRef->MoveTexture);
					entityUnderCursor->SetSelected(true);
				}
				else
				{
					CursorRef->SetCursorTexture(SelectedAbility->CursorIconTexture);
				}
			} break;
			case EAbilityType::E_TARGET_ENEMY:
			{
				if (entityUnderCursor &&
					(entityUnderCursor->Alignment == ETeamAlignment::E_AGGRESSIVE_AI ||
						entityUnderCursor->Alignment == ETeamAlignment::E_ATTACKEVERYTHING_AI))
				{
					CursorRef->SetCursorTexture(CursorRef->AttackMoveTexture);
					entityUnderCursor->SetSelected(true);
				}
				else
				{
					CursorRef->SetCursorTexture(SelectedAbility->CursorIconTexture);
				}
			} break;
			case EAbilityType::E_TARGET_UNIT:
			{
				if (entityUnderCursor)
				{
					CursorRef->SetCursorTexture(CursorRef->MoveTexture);
					entityUnderCursor->SetSelected(true);
				}
				else
				{
					CursorRef->SetCursorTexture(SelectedAbility->CursorIconTexture);
				}
			} break;
			case EAbilityType::E_SELF:
			{
				// Ensure this state never occurs
				checkNoEntry();
			} break;
			case EAbilityType::E_TARGET_GROUND:
			{
				if (!entityUnderCursor)
				{
					UWorld* world = GetWorld();
					FNavLocation navPoint;
					if (world->GetNavigationSystem()->UNavigationSystem::ProjectPointToNavigation(pawnHitResult.Location, navPoint))
					{
						if (CursorRef->CurrentTexture == CursorRef->InvalidTexture)
						{
							CursorRef->SetCursorTexture(SelectedAbility->CursorIconTexture);
						}
					}
					else
					{
						if (CursorRef->CurrentTexture == SelectedAbility->CursorIconTexture)
						{
							CursorRef->SetCursorTexture(CursorRef->InvalidTexture);
						}
					}
				}
				else
				{
					CursorRef->SetCursorTexture(SelectedAbility->CursorIconTexture);
				}
			} break;
			}
		}
		else // No selected ability
		{
			if (m_RTS_GameState->SelectedEntities.Num() > 0)
			{
				if (entityUnderCursor && 
					(entityUnderCursor->Alignment == ETeamAlignment::E_AGGRESSIVE_AI ||
					entityUnderCursor->Alignment == ETeamAlignment::E_ATTACKEVERYTHING_AI))
				{
					CursorRef->SetCursorTexture(CursorRef->AttackMoveTexture);
				}
				else
				{
					if (CursorRef->CurrentTexture == CursorRef->AttackMoveTexture)
					{
						CursorRef->SetCursorTexture(CursorRef->DefaultTexture);
					}
				}
			}
			else
			{
				if (CursorRef->CurrentTexture == CursorRef->AttackMoveTexture)
				{
					CursorRef->SetCursorTexture(CursorRef->DefaultTexture);
				}
			}
		}
	}
}

void ARTS_PlayerController::ActionPrimaryClickPressed()
{
	if (!IsPaused())
	{
		m_ClickStartSS = UGeneralFunctionLibrary_CPP::GetMousePositionVector2D(this);
	}
}

void ARTS_PlayerController::ActionPrimaryClickReleased()
{
	if (!m_RTS_GameState || !m_RTSHUD)
	{
		return;
	}

	if (IsPaused())
	{
		return;
	}

	if (IsCursorOverPurchasableItem())
	{
		// Player clicked on a powerup, don't deselect anything
		CursorRef->SetCursorTexture(CursorRef->DefaultTexture);
		return;
	}

	// Hide selection box when mouse isn't being held
	m_RTSHUD->UpdateSelectionBox(FVector2D::ZeroVector, FVector2D::ZeroVector);

	static ETraceTypeQuery traceType = UEngineTypes::ConvertToTraceType(ECC_Pawn);
	FHitResult hitResult;
	GetHitResultUnderCursorByChannel(traceType, false, hitResult);
	AActor* actorUnderCursor = hitResult.Actor.Get();
	ARTS_Unit* unitUnderCursor = Cast<ARTS_Unit>(actorUnderCursor);
	if (unitUnderCursor)
	{
		if (unitUnderCursor->Health <= 0)
		{
			unitUnderCursor = nullptr; // Don't target dead people
		}
		else if (unitUnderCursor->Immaterial)
		{
			unitUnderCursor = nullptr; // Don't target immaterial people
		}
	}

	const bool shiftIsDown = IsInputKeyDown(FKey("LeftShift"));

	if (SelectedAbility)
	{
		EAbilityType abilityType = SelectedAbility->Type;
		const float abilityRange = SelectedAbility->CastRange;
		if (abilityRange == 0.0f && abilityType != EAbilityType::E_SELF)
		{
			UE_LOG(RTS_PlayerController_Log, Error, TEXT("Ability's cast range is 0! (this is only allowed on abilities whose type is SELF"));
		}

		switch (abilityType)
		{
		case EAbilityType::E_SELF:
		{
			UE_LOG(RTS_PlayerController_Log, Error, TEXT("Ability type is SELF but ability was selected! SELF-targeted abilities should be immediately activated."));
		} break;
		case EAbilityType::E_TARGET_GROUND:
		{
			if (hitResult.bBlockingHit)
			{
				if (m_SpecialistShowingAbilities)
				{
					ARTS_AIController* controller = Cast<ARTS_AIController>(m_SpecialistShowingAbilities->GetController());
					if (IsInputKeyDown(FKey("LeftShift")))
						controller->AddCommand_CastGround(SelectedAbility, hitResult.Location, true, true);
					else
						controller->AddCommand_CastGround(SelectedAbility, hitResult.Location, true, false);

					if (shiftIsDown)
					{
						m_AbilityOnShift = SelectedAbility;
					}
					else
					{
						if (SelectedAbility->Icon)
						{
							SelectedAbility->Icon->OnAbilityActivate();
						}
						SelectedAbility->Deselect();
						SelectedAbility = nullptr;
					}
				}
				else // No valid specialist - don't perform range check
				{
					SelectedAbility->Activate();

					if (shiftIsDown)
					{
						m_AbilityOnShift = SelectedAbility;
					}
					else
					{
						if (SelectedAbility->Icon)
						{
							SelectedAbility->Icon->OnAbilityActivate();
						}
						SelectedAbility->Deselect();
						SelectedAbility = nullptr;
					}
				}
			}
			else
			{
				// TODO: Play sound indicating invalid target location
			}
		} break;
		case EAbilityType::E_TARGET_UNIT:
		{
			if (unitUnderCursor)
			{
				if (m_SpecialistShowingAbilities)
				{
					SelectedAbility->SetTarget(unitUnderCursor);
					unitUnderCursor->SetHighlighted();

					ARTS_AIController* controller = Cast<ARTS_AIController>(m_SpecialistShowingAbilities->GetController());
					if (IsInputKeyDown(FKey("LeftShift")))
						controller->AddCommand_CastTarget(SelectedAbility, Cast<ARTS_Entity>(unitUnderCursor), true, true);
					else
						controller->AddCommand_CastTarget(SelectedAbility, Cast<ARTS_Entity>(unitUnderCursor), true, false);

					if (shiftIsDown)
					{
						m_AbilityOnShift = SelectedAbility;
					}
					else
					{
						if (SelectedAbility->Icon)
						{
							SelectedAbility->Icon->OnAbilityActivate();
						}
						SelectedAbility->Deselect();
						SelectedAbility = nullptr;
					}
				}
				else // No valid specialist - don't perform range check
				{
					SelectedAbility->SetTarget(unitUnderCursor);
					SelectedAbility->Activate();
					unitUnderCursor->SetHighlighted();

					if (shiftIsDown)
					{
						m_AbilityOnShift = SelectedAbility;
					}
					else
					{
						if (SelectedAbility->Icon)
						{
							SelectedAbility->Icon->OnAbilityActivate();
						}
						SelectedAbility->Deselect();
						SelectedAbility = nullptr;
					}
				}
			}
			else
			{
				// TODO: Play sound indicating invalid target location
			}
		} break;
		case EAbilityType::E_TARGET_ALLY:
		{
			if (unitUnderCursor)
			{
				ETeamAlignment entityAlignment = unitUnderCursor->Alignment;
				if (entityAlignment == ETeamAlignment::E_PLAYER)
				{
					if (m_SpecialistShowingAbilities)
					{
						SelectedAbility->SetTarget(unitUnderCursor);
						unitUnderCursor->SetHighlighted();

						ARTS_AIController* controller = Cast<ARTS_AIController>(m_SpecialistShowingAbilities->GetController());
						if (IsInputKeyDown(FKey("LeftShift")))
							controller->AddCommand_CastTarget(SelectedAbility, Cast<ARTS_Entity>(unitUnderCursor), true, true);
						else
							controller->AddCommand_CastTarget(SelectedAbility, Cast<ARTS_Entity>(unitUnderCursor), true, false);

						if (shiftIsDown)
						{
							m_AbilityOnShift = SelectedAbility;
						}
						else
						{
							if (SelectedAbility->Icon)
							{
								SelectedAbility->Icon->OnAbilityActivate();
							}
							SelectedAbility->Deselect();
							SelectedAbility = nullptr;
						}
					}
					else // No valid specialist - don't perform range check
					{
						SelectedAbility->SetTarget(unitUnderCursor);
						SelectedAbility->Activate();
						unitUnderCursor->SetHighlighted();

						if (shiftIsDown)
						{
							m_AbilityOnShift = SelectedAbility;
						}
						else
						{
							if (SelectedAbility->Icon)
							{
								SelectedAbility->Icon->OnAbilityActivate();
							}
							SelectedAbility->Deselect();
							SelectedAbility = nullptr;
						}
					}
				}
				else
				{
					// TODO: Play sound indicating invalid target location
				}
			}
			else
			{
				// TODO: Play sound indicating invalid target location
			}
		} break;
		case EAbilityType::E_TARGET_ENEMY:
		{
			if (unitUnderCursor)
			{
				ETeamAlignment entityAlignment = unitUnderCursor->Alignment;
				if (entityAlignment == ETeamAlignment::E_AGGRESSIVE_AI || entityAlignment == ETeamAlignment::E_ATTACKEVERYTHING_AI)
				{
					if (m_SpecialistShowingAbilities)
					{
						SelectedAbility->SetTarget(unitUnderCursor);
						unitUnderCursor->SetHighlighted();

						ARTS_AIController* controller = Cast<ARTS_AIController>(m_SpecialistShowingAbilities->GetController());
						if (IsInputKeyDown(FKey("LeftShift")))
							controller->AddCommand_CastTarget(SelectedAbility, Cast<ARTS_Entity>(unitUnderCursor), true, true);
						else
							controller->AddCommand_CastTarget(SelectedAbility, Cast<ARTS_Entity>(unitUnderCursor), true, false);

						if (shiftIsDown)
						{
							m_AbilityOnShift = SelectedAbility;
						}
						else
						{
							if (SelectedAbility->Icon)
							{
								SelectedAbility->Icon->OnAbilityActivate();
							}
							SelectedAbility->Deselect();
							SelectedAbility = nullptr;
						}
					}
					else // No valid specialist - don't perform range check
					{
						SelectedAbility->SetTarget(unitUnderCursor);
						SelectedAbility->Activate();
						unitUnderCursor->SetHighlighted();

						if (shiftIsDown)
						{
							m_AbilityOnShift = SelectedAbility;
						}
						else
						{
							if (SelectedAbility->Icon)
							{
								SelectedAbility->Icon->OnAbilityActivate();
							}
							SelectedAbility->Deselect();
							SelectedAbility = nullptr;
						}
					}
				}
				else
				{
					// TODO: Play sound indicating invalid target location
				}
			}
		} break;
		}
	}

	static const auto addToSelectionKeys = PlayerInput->GetKeysForAction("Add To Selection");
	static const FKey addToSelectionKey = addToSelectionKeys[0].Key;
	const bool isAddToSelectionKeyDown = IsInputKeyDown(addToSelectionKey);

	bool doubleClicked = false;
	if (unitUnderCursor)
	{
		float currentTimeSeconds = GetWorld()->GetTimeSeconds();

		if ((unitUnderCursor->Alignment == ETeamAlignment::E_PLAYER ||
			unitUnderCursor->Alignment == ETeamAlignment::E_NEUTRAL_AI) &&
			(currentTimeSeconds - m_LastEntityClickedFrameTime) <= m_DoubleClickPeriodSeconds && m_LastEntityClicked == unitUnderCursor)
		{
			doubleClicked = true;

			for (auto entity : m_RTS_GameState->Entities)
			{
				if (entity->Alignment == unitUnderCursor->Alignment && entity->EntityType == unitUnderCursor->EntityType &&
					entity->IsSelectableByPlayer())
				{
					entity->SetSelected(true);
					m_RTS_GameState->SelectedEntities.AddUnique(entity);
				}
			}
		}

		m_LastEntityClicked = unitUnderCursor;
		m_LastEntityClickedFrameTime = currentTimeSeconds;
	}
	else
	{
		m_LastEntityClicked = nullptr;
	}

	if (!doubleClicked)
	{
		if (!isAddToSelectionKeyDown)
		{
			m_RTS_GameState->SelectedEntities.Empty();
		}

		for (size_t i = 0; i < m_RTS_GameState->Entities.Num(); i++)
		{
			ARTS_Entity* entity = m_RTS_GameState->Entities[i];

			if (entity->IsSelected() && (entity->Alignment == ETeamAlignment::E_PLAYER ||
				entity->Alignment == ETeamAlignment::E_NEUTRAL_AI))
			{
				m_RTS_GameState->SelectedEntities.AddUnique(entity);
			}
		}
	}


	// Ensure specialist whos is showing their ability buttons is still selected
	if (m_SpecialistShowingAbilities)
	{
		if ((m_RTS_GameState->SelectedEntities.Num() != 1) ||
			(m_RTS_GameState->SelectedEntities[0] != m_SpecialistShowingAbilities))
		{
			ClearSpecialistAbilityButtons();
		}
	}

	UpdateSelectedEntitiesBase();
}

void ARTS_PlayerController::ActionSecondaryClickPressed()
{
	static ETraceTypeQuery traceType = UEngineTypes::ConvertToTraceType(ECC_Pawn);
	FHitResult hitResult;
	GetHitResultUnderCursorByChannel(traceType, false, hitResult);
	AActor* actorUnderCursor = hitResult.Actor.Get();
	ARTS_Unit* unitUnderCursor = Cast<ARTS_Unit>(actorUnderCursor);
	if (unitUnderCursor)
	{
		if (unitUnderCursor->Health <= 0)
		{
			unitUnderCursor = nullptr; // Don't target dead people
		}
		else if (unitUnderCursor->Immaterial)
		{
			unitUnderCursor = nullptr; // Don't target immaterial people
		}

		if (m_RTS_GameState->SelectedEntities.Num() > 0)
		{
			if (unitUnderCursor)
				unitUnderCursor->SetHighlighted();
		}
	}
}

void ARTS_PlayerController::ActionSecondaryClickReleased()
{
	//CursorRef->SetCursorTexture(CursorRef->DefaultTexture);
}

void ARTS_PlayerController::ActionTertiaryClickPressed()
{
	if (!AlwaysCenterOnUnits &&
		(!m_MoveToLevelEndAtStartup || (m_MoveToLevelEndAtStartup && m_ReturnedToStartAfterViewingEnd)))
	{
		m_MovingToSelectionCenter = false;
		m_PanMouseStartLocationSSNorm = UGeneralFunctionLibrary_CPP::GetNormalizedMousePosition(this);
		m_PanCamStartLocation = m_RTS_CameraPawn->GetActorLocation();
		m_Panning = true;
	}
}

void ARTS_PlayerController::ActionTertiaryClickReleased()
{
	m_Panning = false;
}

void ARTS_PlayerController::ActionMoveFastPressed()
{
	m_FastMoveMultiplier = m_FastMoveSpeed;
}

void ARTS_PlayerController::ActionMoveFastReleased()
{
	m_FastMoveMultiplier = 1.0f;
}

void ARTS_PlayerController::ActionCenterOnSelectionPressed()
{
	if (!m_RTS_GameState || !m_RTS_CameraPawnCameraComponent)
	{
		return; // Game state hasn't been initialized yet, we can't do anything
	}

	if (m_RTS_GameState->SelectedEntities.Num() == 0)
	{
		return; // No selected entities to center on
	}

	if (!m_MoveToLevelEndAtStartup || (m_MoveToLevelEndAtStartup && m_ReturnedToStartAfterViewingEnd))
	{
		if (!AlwaysCenterOnUnits)
		{
			m_MovingToSelectionCenter = !m_MovingToSelectionCenter;
		}
	}
}

void ARTS_PlayerController::ActionCenterOnSelectionReleased()
{
}

void ARTS_PlayerController::ActionAddToSelectionPressed()
{
	if (m_Panning)
	{
		m_MovingToSelectionCenter = false;
		m_PanMouseStartLocationSSNorm = UGeneralFunctionLibrary_CPP::GetNormalizedMousePosition(this);
		m_PanCamStartLocation = m_RTS_CameraPawn->GetActorLocation();
	}
}

void ARTS_PlayerController::ActionAddToSelectionReleased()
{
	if (m_Panning)
	{
		m_MovingToSelectionCenter = false;
		m_PanMouseStartLocationSSNorm = UGeneralFunctionLibrary_CPP::GetNormalizedMousePosition(this);
		m_PanCamStartLocation = m_RTS_CameraPawn->GetActorLocation();
	}
}

void ARTS_PlayerController::ActionSelectionGroup(int32 Index)
{
	switch (Index)
	{
	case 0:
		ActionSelectionGroup(1, m_RTS_GameState->SelectionGroup1);
		break;
	case 1:
		ActionSelectionGroup(2, m_RTS_GameState->SelectionGroup2);
		break;
	case 2:
		ActionSelectionGroup(3, m_RTS_GameState->SelectionGroup3);
		break;
	case 3:
		ActionSelectionGroup(4, m_RTS_GameState->SelectionGroup4);
		break;
	case 4:
		ActionSelectionGroup(5, m_RTS_GameState->SelectionGroup5);
		break;
	}
}

void ARTS_PlayerController::ActionSelectionGroup(int32 Index, TArray<ARTS_Entity*>& selectionGroupArray)
{
	if (IsPaused())
	{
		return;
	}

	for (int32 i = 0; i < m_RTS_GameState->SelectedEntities.Num(); ++i)
	{
		m_RTS_GameState->SelectedEntities[i]->SetSelected(false);
	}
	ClearSpecialistAbilityButtons();

	m_RTS_GameState->SelectedEntities = selectionGroupArray;

	m_RTSHUD->OnSelectionGroupSelected(Index - 1);

	UpdateSelectedEntitiesBase();
}

void ARTS_PlayerController::ActionCreateSelectionGroup(int32 Index, TArray<ARTS_Entity*>* SelectionGroup)
{
	if (IsPaused())
	{
		return;
	}

	int32 previousSelectionEntityCount = SelectionGroup->Num();
	*SelectionGroup = m_RTS_GameState->SelectedEntities;
	int32 currentSelectionEntityCount = SelectionGroup->Num();
	if (currentSelectionEntityCount == 0)
	{
		if (previousSelectionEntityCount != 0)
		{
			m_RTSHUD->HideSelectionGroupIcon(Index - 1);
		}
	}
	else
	{
		m_RTSHUD->ShowSelectionGroupIcon(Index - 1, currentSelectionEntityCount);
	}
}

void ARTS_PlayerController::ActionSelectionGroup1()
{
	ActionSelectionGroup(1, m_RTS_GameState->SelectionGroup1);
}

void ARTS_PlayerController::ActionCreateSelectionGroup1()
{
	ActionCreateSelectionGroup(1, &m_RTS_GameState->SelectionGroup1);
}

void ARTS_PlayerController::ActionSelectionGroup2()
{
	ActionSelectionGroup(2, m_RTS_GameState->SelectionGroup2);
}

void ARTS_PlayerController::ActionCreateSelectionGroup2()
{
	ActionCreateSelectionGroup(2, &m_RTS_GameState->SelectionGroup2);
}

void ARTS_PlayerController::ActionSelectionGroup3()
{
	ActionSelectionGroup(3, m_RTS_GameState->SelectionGroup3);
}

void ARTS_PlayerController::ActionCreateSelectionGroup3()
{
	ActionCreateSelectionGroup(3, &m_RTS_GameState->SelectionGroup3);
}

void ARTS_PlayerController::ActionSelectionGroup4()
{
	ActionSelectionGroup(4, m_RTS_GameState->SelectionGroup4);
}

void ARTS_PlayerController::ActionCreateSelectionGroup4()
{
	ActionCreateSelectionGroup(4, &m_RTS_GameState->SelectionGroup4);
}

void ARTS_PlayerController::ActionSelectionGroup5()
{
	ActionSelectionGroup(5, m_RTS_GameState->SelectionGroup5);
}

void ARTS_PlayerController::ActionCreateSelectionGroup5()
{
	ActionCreateSelectionGroup(5, &m_RTS_GameState->SelectionGroup5);
}

void ARTS_PlayerController::AxisZoom(float AxisValue)
{
	if (IsPaused())
	{
		return;
	}

	if (m_RTS_CameraPawnSpringArmComponent && AxisValue != 0.0f)
	{
		float deltaArmLength = -AxisValue * m_FastMoveMultiplier * m_ZoomDistance;

		float newArmLength = m_RTS_CameraPawnSpringArmComponent->TargetArmLength + deltaArmLength;
		newArmLength = FMath::Clamp(newArmLength, m_MinArmDistance, m_MaxArmDistance);

		m_TargetZoomArmLength = newArmLength;

		if (!FMath::IsNearlyEqual(m_TargetZoomArmLength, m_RTS_CameraPawnSpringArmComponent->TargetArmLength))
		{
			m_ZoomingToTarget = true;
		}
	}
}

void ARTS_PlayerController::ClearSpecialistAbilityButtons()
{
	if (m_SpecialistShowingAbilities)
	{
		m_SpecialistShowingAbilities->ShowingAbilityIcons = false;
		m_SpecialistShowingAbilities = nullptr;
		m_RTSHUD->ClearSpecialistAbilityIcons();
	}
}

void ARTS_PlayerController::CreateSpecialistAbilityButtons()
{
	if (m_SpecialistShowingAbilities)
	{
		if (!m_SpecialistShowingAbilities->ShowingAbilityIcons)
		{
			m_SpecialistShowingAbilities->ShowingAbilityIcons = true;
			m_RTSHUD->ShowSpecialistAbilityIcons(m_SpecialistShowingAbilities);
		}

		for (int32 i = 0; i < m_SpecialistShowingAbilities->NUM_ABILITIES; ++i)
		{
			m_RTSHUD->UpdateSpecialistAbilityIconProperties(i);
		}
	}
}

void ARTS_PlayerController::UpdateSpecialistAbilityButtons(ARTS_Specialist* SpecialistShowingAbilities)
{
	if (SpecialistShowingAbilities)
	{
		m_SpecialistShowingAbilities = SpecialistShowingAbilities;
	}

	if (m_SpecialistShowingAbilities && m_RTSHUD)
	{
		if (m_SpecialistShowingAbilities->ShowingAbilityIcons)
		{
			for (int32 i = 0; i < m_SpecialistShowingAbilities->NUM_ABILITIES; ++i)
			{
				m_RTSHUD->UpdateSpecialistAbilityIconProperties(i);
			}
		}
	}
}

void ARTS_PlayerController::InvertSelection()
{
	if (IsPaused())
	{
		return;
	}

	if (m_RTS_GameState)
	{
		ClearSpecialistAbilityButtons();

		TArray<ARTS_Entity*> newSelectedEntities;

		for (int32 i = 0; i < m_RTS_GameState->SelectedEntities.Num(); ++i)
		{
			m_RTS_GameState->SelectedEntities[i]->SetSelected(false);
		}

		for (int32 i = 0; i < m_RTS_GameState->Entities.Num(); ++i)
		{
			if ((m_RTS_GameState->Entities[i]->Alignment == ETeamAlignment::E_PLAYER ||
				m_RTS_GameState->Entities[i]->Alignment == ETeamAlignment::E_NEUTRAL_AI) &&
				m_RTS_GameState->Entities[i]->IsSelectableByPlayer() &&
				!m_RTS_GameState->SelectedEntities.Contains(m_RTS_GameState->Entities[i]))
			{
				newSelectedEntities.Push(m_RTS_GameState->Entities[i]);
			}
		}

		m_RTS_GameState->SelectedEntities = newSelectedEntities;

		UpdateSelectedEntitiesBase();
	}
}

URTS_HUDBase* ARTS_PlayerController::GetRTS_HUDBase()
{
	return m_RTSHUD;
}

void ARTS_PlayerController::AddLuma(int32 LumaAmount, bool applyToEndScore)
{
	if (LumaAmount > 0)
	{
		m_CurrentLuma += LumaAmount;
		if (m_RTSHUD)
		{
			m_RTSHUD->UpdateLumaAmount(m_CurrentLuma);
		}
	}

	//Get gamestate & add luma to gamestate
	if (applyToEndScore)
	{
		auto gamestate = GetWorld()->GetGameState<ARTS_GameState>();
		if (gamestate)
		{
			gamestate->LumaGained += LumaAmount;
		}
		else
		{
			UE_LOG(RTS_PlayerController_Log, Error, TEXT("AddLuma > No gamestate present!"))
		}
	}
}

void ARTS_PlayerController::SpendLuma(int32 LumaAmount)
{
	if (LumaAmount > 0 && m_CurrentLuma >= LumaAmount)
	{
		m_CurrentLuma -= LumaAmount;
		if (m_RTSHUD)
		{
			m_RTSHUD->UpdateLumaAmount(m_CurrentLuma);
		}
	}

	//Get gamestate & add luma to gamestate
	auto gamestate = GetWorld()->GetGameState<ARTS_GameState>();
	if (gamestate)
	{
		gamestate->LumaSpent += LumaAmount;
	}
	else
	{
		UE_LOG(RTS_PlayerController_Log, Error, TEXT("SpendLuma > No gamestate present!"))
	}
}

int32 ARTS_PlayerController::GetCurrentLumaAmount()
{
	return m_CurrentLuma;
}

void ARTS_PlayerController::AddHealth(int32 healthAmount)
{
	if (!Team)
	{
		UE_LOG(RTS_PlayerController_Log, Error, TEXT("AddHealth > No player team present!"))
		return;
	}

	if (healthAmount < 0)
	{
		UE_LOG(RTS_PlayerController_Log, Error, TEXT("AddHealth > Health addition can not be smaller than 0"))
		return;
	}

	for (auto entity : Team->Entities)
	{
		entity->Health += healthAmount;
		entity->Health = FMath::Clamp(entity->Health, 1, entity->CurrentDefenceStats.MaxHealth);
	}
	
}

float ARTS_PlayerController::CalculateMovementSpeedBasedOnCameraZoom()
{
	if (m_RTS_CameraPawnSpringArmComponent)
	{
		float movementSpeedMultiplier = m_RTS_CameraPawnSpringArmComponent->TargetArmLength * m_MoveSpeedZoomMultiplier;
		return movementSpeedMultiplier;
	}

	return 0.0f;
}

void ARTS_PlayerController::MoveToCenterOfUnits(bool FocusOnSelectedUnits)
{
	const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	const int32 selectedEntityCount = m_RTS_GameState->SelectedEntities.Num();

	if (FocusOnSelectedUnits && selectedEntityCount == 0)
	{
		m_MovingToSelectionCenter = false;
		return;
	}

	FVector minEntityLocation = FVector::ZeroVector;
	FVector maxEntityLocation = FVector::ZeroVector;
	FVector averageEntityLocation = FVector::ZeroVector;
	static FVector averageEntityForwardLastFrame = FVector::ZeroVector;
	FVector averageEntityForward = FVector::ZeroVector;

	float maxEntityVelocityMag = 0.0f;

	if (FocusOnSelectedUnits)
	{
		for (int32 i = 0; i < selectedEntityCount; ++i)
		{
			ARTS_Entity* entity = m_RTS_GameState->SelectedEntities[i];
			FVector entityLocation = entity->GetActorLocation();
			averageEntityLocation += entityLocation;
			averageEntityForward += entity->GetActorForwardVector();

			FVector entityLocationCopy = entityLocation;

			UGeneralFunctionLibrary_CPP::FVectorMinMax(minEntityLocation, entityLocationCopy);
			entityLocationCopy = entityLocation;
			UGeneralFunctionLibrary_CPP::FVectorMinMax(entityLocationCopy, maxEntityLocation);

			const float entityVelocityMag = entity->GetVelocity().Size();
			if (entityVelocityMag > maxEntityVelocityMag)
			{
				maxEntityVelocityMag = entityVelocityMag;
			}
		}

		averageEntityForward.Normalize();
		averageEntityLocation /= selectedEntityCount;
	}
	else
	{
		int32 entityCount = 0;

		for (int32 i = 0; i < m_RTS_GameState->Entities.Num(); ++i)
		{
			ARTS_Entity* entity = m_RTS_GameState->Entities[i];
			if (entity && entity->IsSelectableByPlayer())
			{
				++entityCount;

				FVector entityLocation = entity->GetActorLocation();
				averageEntityLocation += entityLocation;
				averageEntityForward += entity->GetActorForwardVector();

				FVector entityLocationCopy = entityLocation;

				UGeneralFunctionLibrary_CPP::FVectorMinMax(minEntityLocation, entityLocationCopy);
				entityLocationCopy = entityLocation;
				UGeneralFunctionLibrary_CPP::FVectorMinMax(entityLocationCopy, maxEntityLocation);

				const float entityVelocityMag = entity->GetVelocity().Size();
				if (entityVelocityMag > maxEntityVelocityMag)
				{
					maxEntityVelocityMag = entityVelocityMag;
				}
			}
		}

		if (entityCount > 0)
		{
			averageEntityForward.Normalize();
			averageEntityLocation /= entityCount;
		}
	}

	FVector camForward = m_RTS_CameraPawnMeshComponent->GetForwardVector();
	camForward.Z = 0; // Only move along XY plane
	camForward.Normalize();
	// Shift the target position up to account for the HUD at the bottom of the screen
	
	float yOffset = m_RTS_CameraPawnSpringArmComponent->TargetArmLength * m_CenterOffsetY;
	FVector oldCameraLocation = m_RTS_CameraPawn->GetActorLocation();
	m_TargetLocation = FVector(
		averageEntityLocation.X, 
		averageEntityLocation.Y, 
		oldCameraLocation.Z); // NOTE: Camera height is not effected here (it is handled with zooming)
	m_TargetLocation -= camForward * yOffset;

	averageEntityForward = FMath::Lerp(averageEntityForwardLastFrame, averageEntityForward, m_CenterOffsetEntitySpeed);
	if (averageEntityForward != FVector::ZeroVector)
	{
		m_TargetLocation += averageEntityForward * m_RTS_CameraPawnSpringArmComponent->TargetArmLength * m_CenterOffsetEntityDirection;
	}

	averageEntityForwardLastFrame = averageEntityForward;

	FVector dCamLocation = m_TargetLocation - oldCameraLocation;
	FVector camMovement = dCamLocation * m_SelectionCenterMaxMoveSpeed * DeltaSeconds;

	FVector newCamLocation = oldCameraLocation + camMovement;

	if (newCamLocation.X > m_TargetLocation.X && dCamLocation.X > 0.0f ||
		newCamLocation.X < m_TargetLocation.X && dCamLocation.X < 0.0f)
	{
		// Passed the target in the X direction
		newCamLocation.X = m_TargetLocation.X;
	}
	if (newCamLocation.Y > m_TargetLocation.Y && dCamLocation.Y > 0.0f ||
		newCamLocation.Y < m_TargetLocation.Y && dCamLocation.Y < 0.0f)
	{
		// Passed the target in the Y direction
		newCamLocation.Y = m_TargetLocation.Y;
	}
	if (newCamLocation.Z > m_TargetLocation.Z && dCamLocation.Z > 0.0f ||
		newCamLocation.Z < m_TargetLocation.Z && dCamLocation.Z < 0.0f)
	{
		// Passed the target in the Z direction
		newCamLocation.Z = m_TargetLocation.Z;
	}
	
	newCamLocation = ClampCamPosWithBounds(newCamLocation);

	m_RTS_CameraPawn->SetActorLocation(newCamLocation);
}

void ARTS_PlayerController::MoveToTarget()
{
	const float DeltaSeconds = GetWorld()->GetDeltaSeconds();

	FVector oldCameraLocation = m_RTS_CameraPawn->GetActorLocation();
	m_TargetLocation = FVector(m_TargetLocation.X, m_TargetLocation.Y,
		oldCameraLocation.Z); // NOTE: Don't change the height of the camera here (handled with zooming)
	FVector dCamLocation = m_TargetLocation - oldCameraLocation;
	FVector camMovement = dCamLocation * m_SelectionCenterMaxMoveSpeed * DeltaSeconds * m_MoveToTargetSpeed;

	FVector newCamLocation = oldCameraLocation + camMovement;

	if (newCamLocation.X > m_TargetLocation.X && dCamLocation.X > 0.0f ||
		newCamLocation.X < m_TargetLocation.X && dCamLocation.X < 0.0f)
	{
		// We passed the target in the X direction
		newCamLocation.X = m_TargetLocation.X;
	}
	if (newCamLocation.Y > m_TargetLocation.Y && dCamLocation.Y > 0.0f ||
		newCamLocation.Y < m_TargetLocation.Y && dCamLocation.Y < 0.0f)
	{
		// We passed the target in the Y direction
		newCamLocation.Y = m_TargetLocation.Y;
	}
	if (newCamLocation.Z > m_TargetLocation.Z && dCamLocation.Z > 0.0f ||
		newCamLocation.Z < m_TargetLocation.Z && dCamLocation.Z < 0.0f)
	{
		// We passed the target in the Z direction
		newCamLocation.Z = m_TargetLocation.Z;
	}

	newCamLocation = ClampCamPosWithBounds(newCamLocation);

	m_RTS_CameraPawn->SetActorLocation(newCamLocation);

	float locationTolerance = 300.0f;
	if (m_TargetLocation.Equals(oldCameraLocation, locationTolerance))
	{
		m_MovingToTarget = false;

		if (m_MoveToLevelEndAtStartup && m_TargetLocation.Equals(m_LevelStartLocation, 0.1f))
		{
			m_ReturnedToStartAfterViewingEnd = true;
		}
	}
}

void ARTS_PlayerController::StartMovingToLevelEnd()
{
	m_TargetLocation = m_LevelEndLocation;
	m_MovingToTarget = true;
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARTS_PlayerController::StartMovingToLevelStart, m_DelayBeforeMovingBackToLevelStart, false);
}

void ARTS_PlayerController::StartMovingToLevelStart()
{
	m_TargetLocation = m_LevelStartLocation;
	m_MovingToTarget = true;
}

FVector ARTS_PlayerController::ClampCamPosWithBounds(FVector camPos)
{
	if (m_LevelBounds)
	{
		FVector boundsExtent = m_LevelBounds->BoxComponent->GetScaledBoxExtent();
		FVector boundsCenter = m_LevelBounds->BoxComponent->GetComponentLocation();
		if (camPos.X > boundsCenter.X + boundsExtent.X)
		{
			camPos.X = (boundsCenter.X + boundsExtent.X);
		}
		else if (camPos.X < boundsCenter.X - boundsExtent.X)
		{
			camPos.X = (boundsCenter.X - boundsExtent.X);
		}

		if (camPos.Y > boundsCenter.Y + boundsExtent.Y)
		{
			camPos.Y = (boundsCenter.Y + boundsExtent.Y);
		}
		else if (camPos.Y < boundsCenter.Y - boundsExtent.Y)
		{
			camPos.Y = (boundsCenter.Y - boundsExtent.Y);
		}
	}

	return camPos;
}

bool ARTS_PlayerController::IsCursorOverPurchasableItem()
{
	static ETraceTypeQuery visibilityTraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	FHitResult visibilityHitResult;
	GetHitResultUnderCursorByChannel(visibilityTraceType, false, visibilityHitResult);
	AActor* otherActorUnderCursor = visibilityHitResult.GetActor();
	if (otherActorUnderCursor && otherActorUnderCursor->ActorHasTag("Purchasable"))
	{
		return true;
	}
	return false;
}

FVector ARTS_PlayerController::ClampDCamPosWithBounds(FVector dCamPos)
{
	if (m_LevelBounds)
	{
		FVector pCamPos = m_RTS_CameraPawn->GetActorLocation();
		FVector newCamPos = pCamPos + dCamPos;
		newCamPos = ClampCamPosWithBounds(newCamPos);

		dCamPos = newCamPos - pCamPos;
	}

	return dCamPos;
}
