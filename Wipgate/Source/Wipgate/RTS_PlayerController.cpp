// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_PlayerController.h"

#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/StaticMeshComponent.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Engine/UserInterfaceSettings.h"
#include "Engine/RendererSettings.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Ability.h"
#include "AbilityIcon.h"
#include "RTS_GameState.h"
#include "RTS_HUDBase.h"
#include "RTS_Entity.h"
#include "RTS_Unit.h"
#include "RTS_Specialist.h"
#include "RTS_Squad.h"
#include "GeneralFunctionLibrary_CPP.h"

DEFINE_LOG_CATEGORY_STATIC(RTS_PlayerController_Log, Log, All);

ARTS_PlayerController::ARTS_PlayerController()
{
}

void ARTS_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Get references to camera and its components
	m_RTS_CameraPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	check(m_RTS_CameraPawn != nullptr);

	m_RTS_CameraPawn->SetActorLocation(m_StartingLocation);
	m_RTS_CameraPawn->SetActorRotation(m_StartingRotation);

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

	AGameStateBase* baseGameState = GetWorld()->GetGameState();
	m_RTS_GameState = Cast<ARTS_GameState>(baseGameState);
	check(m_RTS_GameState != nullptr);

	// Set input mode to show cursor when captured (clicked) and to lock cursor to viewport
	FInputModeGameAndUI inputMode;
	inputMode.SetHideCursorDuringCapture(false);
	inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(inputMode);

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

		bShowMouseCursor = true;
	}
	else
	{
		UE_LOG(RTS_PlayerController_Log, Error, TEXT("Main HUD template was not set in player controller BP!"));
	}

	if (m_RTSHUD)
	{
		m_RTSHUD->AddSelectionGroupIconsToGrid(SELECTION_GROUP_COUNT);
	}
}

void ARTS_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Primary Click", IE_Pressed, this, &ARTS_PlayerController::ActionPrimaryClickPressed);
	InputComponent->BindAction("Primary Click", IE_Released, this, &ARTS_PlayerController::ActionPrimaryClickReleased);
	InputComponent->BindAction("Secondary Click", IE_Pressed, this, &ARTS_PlayerController::ActionSecondaryClickPressed);
	InputComponent->BindAction("Secondary Click", IE_Released, this, &ARTS_PlayerController::ActionSecondaryClickReleased);
	InputComponent->BindAction("Move Fast", IE_Pressed, this, &ARTS_PlayerController::ActionMoveFastPressed);
	InputComponent->BindAction("Move Fast", IE_Released, this, &ARTS_PlayerController::ActionMoveFastReleased);

	InputComponent->BindAction("Ability Active", IE_Released, this, &ARTS_PlayerController::OnAbilityActiveButtonPress);
	InputComponent->BindAction("Ability Construct", IE_Released, this, &ARTS_PlayerController::OnAbilityConstructButtonPress);
	InputComponent->BindAction("Ability Passive", IE_Released, this, &ARTS_PlayerController::OnAbilityPassiveButtonPress);

	InputComponent->BindAction("Selection Group 1", IE_Released, this, &ARTS_PlayerController::ActionSelectionGroup1);
	InputComponent->BindAction("Create Selection Group 1", IE_Released, this, &ARTS_PlayerController::ActionCreateSelectionGroup1);
	InputComponent->BindAction("Selection Group 2", IE_Released, this, &ARTS_PlayerController::ActionSelectionGroup2);
	InputComponent->BindAction("Create Selection Group 2", IE_Released, this, &ARTS_PlayerController::ActionCreateSelectionGroup2);
	InputComponent->BindAction("Selection Group 3", IE_Released, this, &ARTS_PlayerController::ActionSelectionGroup3);
	InputComponent->BindAction("Create Selection Group 3", IE_Released, this, &ARTS_PlayerController::ActionCreateSelectionGroup3);
	InputComponent->BindAction("Selection Group 4", IE_Released, this, &ARTS_PlayerController::ActionSelectionGroup4);
	InputComponent->BindAction("Create Selection Group 4", IE_Released, this, &ARTS_PlayerController::ActionCreateSelectionGroup4);
	InputComponent->BindAction("Selection Group 5", IE_Released, this, &ARTS_PlayerController::ActionSelectionGroup5);
	InputComponent->BindAction("Create Selection Group 5", IE_Released, this, &ARTS_PlayerController::ActionCreateSelectionGroup5);

	InputComponent->BindAxis("Zoom", this, &ARTS_PlayerController::AxisZoom);
	InputComponent->BindAxis("Move Right", this, &ARTS_PlayerController::AxisMoveRight);
	InputComponent->BindAxis("Move Forward", this, &ARTS_PlayerController::AxisMoveForward);
}

bool ARTS_PlayerController::IsEdgeMovementEnabled() const
{
	return m_EdgeMovementEnabled;
}

void ARTS_PlayerController::SetEdgeMovementEnabled(bool enabled)
{
	m_EdgeMovementEnabled = enabled;
}

void ARTS_PlayerController::Tick(float DeltaSeconds)
{
	if (!m_RTS_GameState ||!m_RTSHUD)
	{
		return;
	}

	if (IsInputKeyDown(EKeys::Escape))
	{
		GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
		return;
	}

	// Update selection box size if mouse is being dragged
	if (!SelectedAbility && IsInputKeyDown(EKeys::LeftMouseButton))
	{
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

	// TODO: Find out how to use input mapped key
	if (IsInputKeyDown(EKeys::SpaceBar))
	{
		ActionCenterOnSelection();
	}
	else if (m_MovingToTarget)
	{
		MoveToTarget();
	}
	// If mouse is at edge of screen, update camera pos
	/*
		These two values need to be checked because Tick can be called
		before they are set in BeginPlay somehow
		TODO: Look into call order - I think this function even gets called in the editor for some reason
	*/
	else if (m_EdgeMovementEnabled && m_RTS_CameraPawnMeshComponent && m_RTS_CameraPawn)
	{
		FVector rightVec = m_RTS_CameraPawnMeshComponent->GetRightVector();
		FVector forwardVec = m_RTS_CameraPawnMeshComponent->GetForwardVector();
		forwardVec.Z = 0; // Only move along XY plane
		forwardVec.Normalize();

		float camDistSpeedMultiplier = 0.0f;
		UWorld* world = GetWorld();
		if (!world)
		{
			UE_LOG(RTS_PlayerController_Log, Error, TEXT("World not found!"));
		}
		else
		{
			camDistSpeedMultiplier = CalculateMovementSpeedBasedOnCameraZoom(world->DeltaTimeSeconds);
		}

		FVector2D normMousePos = UGeneralFunctionLibrary_CPP::GetNormalizedMousePosition(this);
		if (normMousePos.X > 1.0f - m_EdgeSize)
		{
			m_RTS_CameraPawn->AddActorWorldOffset(rightVec * m_EdgeMoveSpeed * m_FastMoveMultiplier * camDistSpeedMultiplier);
		}
		else if (normMousePos.X < m_EdgeSize)
		{
			m_RTS_CameraPawn->AddActorWorldOffset(-rightVec * m_EdgeMoveSpeed * m_FastMoveMultiplier * camDistSpeedMultiplier);
		}

		if (normMousePos.Y > 1.0f - m_EdgeSize)
		{
			m_RTS_CameraPawn->AddActorWorldOffset(-forwardVec * m_EdgeMoveSpeed * m_FastMoveMultiplier * camDistSpeedMultiplier);
		}
		else if (normMousePos.Y < m_EdgeSize)
		{
			m_RTS_CameraPawn->AddActorWorldOffset(forwardVec * m_EdgeMoveSpeed * m_FastMoveMultiplier * camDistSpeedMultiplier);
		}
	}


	bool showViewportOnMinimap = true;
	if (showViewportOnMinimap)
	{
		// TODO: Implement
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

	static ETraceTypeQuery traceType = UEngineTypes::ConvertToTraceType(ECC_Pawn);
	FHitResult hitResult;
	GetHitResultUnderCursorByChannel(traceType, false, hitResult);
	AActor* actorUnderCursor = hitResult.Actor.Get();
	ARTS_Entity* entityUnderCursor = nullptr;

	FVector2D selectionBoxMin = m_ClickStartSS;
	FVector2D selectionBoxMax = m_ClickEndSS;
	UGeneralFunctionLibrary_CPP::FVector2DMinMax(selectionBoxMin, selectionBoxMax);

	FVector2D viewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	const UUserInterfaceSettings* uiSettings = GetDefault<UUserInterfaceSettings>();
	float viewportScale = uiSettings->GetDPIScaleBasedOnSize(FIntPoint((int)viewportSize.X, (int)viewportSize.Y));

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
			FVector entityBoundsMinWS = entityLocation - entity->SelectionHitBox;
			FVector2D entityBoundsMinSS;
			ProjectWorldLocationToScreen(entityBoundsMinWS, entityBoundsMinSS, true);

			FVector entityBoundsMaxWS = entityLocation + entity->SelectionHitBox;
			FVector2D entityBoundsMaxSS;
			ProjectWorldLocationToScreen(entityBoundsMaxWS, entityBoundsMaxSS, true);

			UGeneralFunctionLibrary_CPP::FVector2DMinMax(entityBoundsMinSS, entityBoundsMaxSS);

			entityBoundsMinSS /= viewportScale;
			entityBoundsMaxSS /= viewportScale;

			entityInSelectionBox =
				UGeneralFunctionLibrary_CPP::PointInBounds2D(entityBoundsMinSS, selectionBoxMin, selectionBoxMax) ||
				UGeneralFunctionLibrary_CPP::PointInBounds2D(entityBoundsMaxSS, selectionBoxMin, selectionBoxMax);
		}

		// Check if entity is under mouse cursor (for single clicks)
		bool isThisUnitUnderCursor = (actorUnderCursor == entity);

		if (isThisUnitUnderCursor)
		{
			entityUnderCursor = entity;
		}

		const bool entityIsDead = entity->CurrentDefenceStats.Health <= 0;

		const bool entityWasSelected = entity->IsSelected();
		bool entityDeselected = isThisUnitUnderCursor && isAddToSelectionKeyDown && entityWasSelected && isPrimaryClickButtonClicked;
		bool entityWasLikelyDeselectedLastFrame = isThisUnitUnderCursor && isAddToSelectionKeyDown && isPrimaryClickButtonDown && !isPrimaryClickButtonClicked && !entityWasSelected;

		if (!SelectedAbility && entity->Team.Alignment == ETeamAlignment::E_FRIENDLY)
		{
			if (!entityIsDead)
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
		if (m_SpecialistShowingAbilities->CurrentDefenceStats.Health <= 0)
		{
			ClearAbilityButtons();
			m_RTS_GameState->SelectedEntities.Empty();
			m_RTS_GameState->Entities.Remove(m_SpecialistShowingAbilities);
			m_SpecialistShowingAbilities = nullptr;
		}
		else
		{
			UpdateAbilityButtons();
		}
	}

	// Update squads
	for (URTS_Squad* squad : m_Squads)
	{
		if (squad->Units.Num() <= 0)
		{
			m_Squads.Remove(squad);
			continue;
		}

		squad->Update(DeltaSeconds);
	}
}

void ARTS_PlayerController::ActionPrimaryClickPressed()
{
	m_ClickStartSS = UGeneralFunctionLibrary_CPP::GetMousePositionVector2D(this);
}

void ARTS_PlayerController::ActionPrimaryClickReleased()
{
	if (!m_RTS_GameState || !m_RTSHUD)
	{
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
		if (unitUnderCursor->CurrentDefenceStats.Health <= 0)
		{
			unitUnderCursor = nullptr; // Don't target dead people
		}
		else if (unitUnderCursor->Immaterial)
		{
			unitUnderCursor = nullptr; // Don't target immaterial people
		}
	}

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
				if (!m_SpecialistShowingAbilities)
				{
					UE_LOG(RTS_PlayerController_Log, Error, TEXT("Specialist showing abilities not set before ground click!"));
				}
				else
				{
					float entityDist = FVector::DistXY(hitResult.ImpactPoint, m_SpecialistShowingAbilities->GetActorLocation());
					if (entityDist < abilityRange)
					{
						SelectedAbility->Activate();
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
				if (!m_SpecialistShowingAbilities)
				{
					UE_LOG(RTS_PlayerController_Log, Error, TEXT("Specialist showing abilities not set before entity click!"));
				}
				else
				{
					float entityDist = FVector::DistXY(unitUnderCursor->GetActorLocation(), m_SpecialistShowingAbilities->GetActorLocation());
					if (entityDist < abilityRange)
					{
						SelectedAbility->SetTarget(unitUnderCursor);
						SelectedAbility->Activate();
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
				ETeamAlignment entityAlignment = unitUnderCursor->Team.Alignment;
				if (entityAlignment == ETeamAlignment::E_FRIENDLY)
				{
					if (!m_SpecialistShowingAbilities)
					{
						UE_LOG(RTS_PlayerController_Log, Error, TEXT("Specialist showing abilities not set ally ground click!"));
					}
					else
					{
						float entityDist = FVector::DistXY(unitUnderCursor->GetActorLocation(), m_SpecialistShowingAbilities->GetActorLocation());
						if (entityDist < abilityRange)
						{
							SelectedAbility->SetTarget(unitUnderCursor);
							SelectedAbility->Activate();
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
				ETeamAlignment entityAlignment = unitUnderCursor->Team.Alignment;
				if (entityAlignment == ETeamAlignment::E_ENEMY)
				{
					if (!m_SpecialistShowingAbilities)
					{
						UE_LOG(RTS_PlayerController_Log, Error, TEXT("Specialist showing abilities not set before enemy click!"));
					}
					else
					{
						float entityDist = FVector::DistXY(unitUnderCursor->GetActorLocation(), m_SpecialistShowingAbilities->GetActorLocation());
						if (entityDist < abilityRange)
						{
							SelectedAbility->SetTarget(unitUnderCursor);
							SelectedAbility->Activate();
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

	const int32 prevNumEntitiesSelected = m_RTS_GameState->SelectedEntities.Num();

	bool doubleClicked = false;
	if (unitUnderCursor)
	{
		float currentTimeSeconds = GetWorld()->GetTimeSeconds();

		if ((currentTimeSeconds - m_LastEntityClickedFrameTime) <= m_DoubleClickPeriodSeconds && m_LastEntityClicked == unitUnderCursor)
		{
			doubleClicked = true;
			float targetRange = unitUnderCursor->CurrentAttackStats.Range;

			for (auto entity : m_RTS_GameState->Entities)
			{
				if (entity->Team.Alignment == ETeamAlignment::E_FRIENDLY && entity->CurrentAttackStats.Range == targetRange)
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

			if (entity->IsSelected())
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
			ClearAbilityButtons();
		}
	}

	m_RTSHUD->UpdateSelectedEntities(m_RTS_GameState->SelectedEntities);

	if (m_RTS_GameState->SelectedEntities.Num() == 1)
	{
		if (!SelectedAbility && !m_SpecialistShowingAbilities)
		{
			ARTS_Entity* entity = m_RTS_GameState->SelectedEntities[0];

			ARTS_Specialist* specialist = Cast<ARTS_Specialist>(entity);
			if (specialist)
			{
				m_SpecialistShowingAbilities = specialist;
				CreateAbilityButtons();
			}
		}

		m_RTSHUD->ShowSelectedUnitStats(m_RTS_GameState->SelectedEntities[0]);
	}
	else
	{
		if (prevNumEntitiesSelected == 1)
		{
			m_RTSHUD->HideSelectedUnitStats();
		}
	}

}

void ARTS_PlayerController::ActionSecondaryClickPressed()
{
	if (SelectedAbility)
	{
		SelectedAbility->Deselect();
		SelectedAbility = nullptr;
	}
}

void ARTS_PlayerController::ActionSecondaryClickReleased()
{
}

void ARTS_PlayerController::ActionMoveFastPressed()
{
	m_FastMoveMultiplier = m_FastMoveSpeed;
}

void ARTS_PlayerController::ActionMoveFastReleased()
{
	m_FastMoveMultiplier = 1.0f;
}

void ARTS_PlayerController::ActionCenterOnSelection()
{
	if (!m_RTS_GameState || !m_RTS_CameraPawnCameraComponent)
	{
		return; // Game state hasn't been initialized yet, we can't do anything
	}

	if (m_RTS_GameState->SelectedEntities.Num() == 0)
	{
		return; // No selected entities to center on
	}

	m_MovingToTarget = true;
	MoveToTarget();
}

void ARTS_PlayerController::ActionSelectionGroup(int32 Index)
{
	switch (Index)
	{
	case 0:
		ActionSelectionGroup(m_RTS_GameState->SelectionGroup1);
		break;
	case 1:
		ActionSelectionGroup(m_RTS_GameState->SelectionGroup2);
		break;
	case 2:
		ActionSelectionGroup(m_RTS_GameState->SelectionGroup3);
		break;
	case 3:
		ActionSelectionGroup(m_RTS_GameState->SelectionGroup4);
		break;
	case 4:
		ActionSelectionGroup(m_RTS_GameState->SelectionGroup5);
		break;
	}
}

URTS_Squad* ARTS_PlayerController::AddSquad()
{
	URTS_Squad* squad = NewObject<URTS_Squad>(this);
	m_Squads.Push(squad);
	return squad;
}

void ARTS_PlayerController::ActionSelectionGroup(TArray<ARTS_Entity*>& selectionGroupArray)
{
	for (int32 i = 0; i < m_RTS_GameState->SelectedEntities.Num(); ++i)
	{
		m_RTS_GameState->SelectedEntities[i]->SetSelected(false);
	}
	ClearAbilityButtons();

	m_RTS_GameState->SelectedEntities = selectionGroupArray;

	for (int32 i = 0; i < m_RTS_GameState->SelectedEntities.Num(); ++i)
	{
		if (m_RTS_GameState->SelectedEntities[i]->CurrentDefenceStats.Health > 0)
		{
			m_RTS_GameState->SelectedEntities[i]->SetSelected(true);
		}
	}

	if (m_RTS_GameState->SelectedEntities.Num() == 1)
	{
		ARTS_Entity* entity = m_RTS_GameState->SelectedEntities[0];
		ARTS_Specialist* specialist = Cast<ARTS_Specialist>(entity);
		if (specialist && specialist->CurrentDefenceStats.Health > 0)
		{
			m_SpecialistShowingAbilities = specialist;
			CreateAbilityButtons();
		}
	}

	m_RTSHUD->UpdateSelectedEntities(m_RTS_GameState->SelectedEntities);
}

void ARTS_PlayerController::ActionCreateSelectionGroup(int32 Index, TArray<ARTS_Entity*>* SelectionGroup, bool* SelectionGroupIconCreated)
{
	*SelectionGroup = m_RTS_GameState->SelectedEntities;
	if (m_RTS_GameState->SelectedEntities.Num() == 0)
	{
		m_RTSHUD->HideSelectionGroupIcon(Index - 1);
		*SelectionGroupIconCreated = false;
	}
	else
	{
		if (!(*SelectionGroupIconCreated))
		{
			m_RTSHUD->ShowSelectionGroupIcon(Index - 1);
			*SelectionGroupIconCreated = true;
		}
	}
}

void ARTS_PlayerController::ActionSelectionGroup1()
{
	ActionSelectionGroup(m_RTS_GameState->SelectionGroup1);
}

void ARTS_PlayerController::ActionCreateSelectionGroup1()
{
	ActionCreateSelectionGroup(1, &m_RTS_GameState->SelectionGroup1, &m_RTS_GameState->SelectionGroup1IconCreated);
}

void ARTS_PlayerController::ActionSelectionGroup2()
{
	ActionSelectionGroup(m_RTS_GameState->SelectionGroup2);
}

void ARTS_PlayerController::ActionCreateSelectionGroup2()
{
	ActionCreateSelectionGroup(2, &m_RTS_GameState->SelectionGroup2, &m_RTS_GameState->SelectionGroup2IconCreated);
}

void ARTS_PlayerController::ActionSelectionGroup3()
{
	ActionSelectionGroup(m_RTS_GameState->SelectionGroup3);
}

void ARTS_PlayerController::ActionCreateSelectionGroup3()
{
	ActionCreateSelectionGroup(3, &m_RTS_GameState->SelectionGroup3, &m_RTS_GameState->SelectionGroup3IconCreated);
}

void ARTS_PlayerController::ActionSelectionGroup4()
{
	ActionSelectionGroup(m_RTS_GameState->SelectionGroup4);
}

void ARTS_PlayerController::ActionCreateSelectionGroup4()
{
	ActionCreateSelectionGroup(4, &m_RTS_GameState->SelectionGroup4, &m_RTS_GameState->SelectionGroup4IconCreated);
}

void ARTS_PlayerController::ActionSelectionGroup5()
{
	ActionSelectionGroup(m_RTS_GameState->SelectionGroup5);
}

void ARTS_PlayerController::ActionCreateSelectionGroup5()
{
	ActionCreateSelectionGroup(5, &m_RTS_GameState->SelectionGroup5, &m_RTS_GameState->SelectionGroup5IconCreated);
}

void ARTS_PlayerController::AxisZoom(float AxisValue)
{
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

void ARTS_PlayerController::AxisMoveRight(float AxisValue)
{
	if (m_RTS_CameraPawn && m_RTS_CameraPawnMeshComponent && AxisValue != 0.0f)
	{
		float camDistSpeedMultiplier = CalculateMovementSpeedBasedOnCameraZoom(GetWorld()->DeltaTimeSeconds);

		FVector rightVec = m_RTS_CameraPawnMeshComponent->GetRightVector();

		m_RTS_CameraPawn->AddActorWorldOffset(rightVec * AxisValue * camDistSpeedMultiplier * m_FastMoveMultiplier);
	}
}

void ARTS_PlayerController::ClearAbilityButtons()
{
	if (m_SpecialistShowingAbilities)
	{
		m_SpecialistShowingAbilities->ShowingAbilityIcons = false;
		m_SpecialistShowingAbilities = nullptr;
		m_RTSHUD->ClearAbilityIconsFromCommandCardGrid();
	}
}

void ARTS_PlayerController::CreateAbilityButtons()
{
	if (m_SpecialistShowingAbilities)
	{
		if (!m_SpecialistShowingAbilities->ShowingAbilityIcons)
		{
			m_SpecialistShowingAbilities->ShowingAbilityIcons = true;
			for (int32 i = 0; i < m_SpecialistShowingAbilities->NUM_ABILITIES; ++i)
			{
				m_RTSHUD->AddAbilityIconToCommandCardGrid(m_SpecialistShowingAbilities);
			}
		}

		for (int32 i = 0; i < m_SpecialistShowingAbilities->NUM_ABILITIES; ++i)
		{
			int col = i;
			int row = 1;
			FLinearColor progressBarBGCol = (i == 0 ? FLinearColor::Red : (i == 1 ? FLinearColor::Blue : FLinearColor::Green));
			FLinearColor buttonBGCol = progressBarBGCol.Desaturate(0.5f);

			m_RTSHUD->UpdateAbilityIconProperties(i, col, row, buttonBGCol, progressBarBGCol);
		}
	}
}

void ARTS_PlayerController::UpdateAbilityButtons(ARTS_Specialist* SpecialistShowingAbilities)
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
				int col = i;
				int row = 1;
				FLinearColor progressBarBGCol = (i == 0 ? FLinearColor::Red : (i == 1 ? FLinearColor::Blue : FLinearColor::Green));;
				FLinearColor buttonBGCol = progressBarBGCol.Desaturate(0.5f);

				m_RTSHUD->UpdateAbilityIconProperties(i, col, row, buttonBGCol, progressBarBGCol);
			}
		}
	}
}

URTS_HUDBase* ARTS_PlayerController::GetHUD()
{
	return m_RTSHUD;
}

void ARTS_PlayerController::AxisMoveForward(float AxisValue)
{
	if (m_RTS_CameraPawn && AxisValue != 0.0f)
	{
		float camDistSpeedMultiplier = CalculateMovementSpeedBasedOnCameraZoom(GetWorld()->DeltaTimeSeconds);

		FVector forwardVec = m_RTS_CameraPawnMeshComponent->GetForwardVector();
		forwardVec.Z = 0; // Only move along XY plane
		forwardVec.Normalize();

		m_RTS_CameraPawn->AddActorWorldOffset(forwardVec * AxisValue * camDistSpeedMultiplier * m_FastMoveMultiplier);
	}
}

float ARTS_PlayerController::CalculateMovementSpeedBasedOnCameraZoom(float DeltaSeconds)
{
	if (m_RTS_CameraPawnSpringArmComponent)
	{
		float movementSpeedMultiplier = m_RTS_CameraPawnSpringArmComponent->TargetArmLength * m_MoveSpeedZoomMultiplier * DeltaSeconds;
		return movementSpeedMultiplier;
	}

	return 0.0f;
}

void ARTS_PlayerController::MoveToTarget()
{
	const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	const int32 selectedEntityCount = m_RTS_GameState->SelectedEntities.Num();

	if (selectedEntityCount == 0)
	{
		m_MovingToTarget = false;
		return;
	}

	FVector minEntityLocation = FVector::ZeroVector;
	FVector maxEntityLocation = FVector::ZeroVector;
	FVector averageEntityLocation = FVector::ZeroVector;

	float maxEntityVelocityMag = 0.0f;

	for (int32 i = 0; i < selectedEntityCount; ++i)
	{
		ARTS_Entity* entity = m_RTS_GameState->SelectedEntities[i];
		FVector entityLocation = entity->GetActorLocation();
		averageEntityLocation += entityLocation;

		FVector entityLocationCopy = entityLocation; // TODO: Is this needed?

		UGeneralFunctionLibrary_CPP::FVectorMinMax(minEntityLocation, entityLocationCopy);
		entityLocationCopy = entityLocation;
		UGeneralFunctionLibrary_CPP::FVectorMinMax(entityLocationCopy, maxEntityLocation);

		const float entityVelocityMag = entity->GetVelocity().Size();
		if (entityVelocityMag > maxEntityVelocityMag)
		{
			maxEntityVelocityMag = entityVelocityMag;
		}
	}

	averageEntityLocation /= selectedEntityCount;

	FVector oldCameraLocation = m_RTS_CameraPawn->GetActorLocation();
	m_TargetLocation = FVector(averageEntityLocation.X, averageEntityLocation.Y, 
		oldCameraLocation.Z); // NOTE: Don't change the height of the camera here (handled with zooming)
	FVector dCamLocation = m_TargetLocation - oldCameraLocation;
	FVector camMovement = dCamLocation * m_SelectionCenterMaxMoveSpeed * DeltaSeconds;

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

	m_RTS_CameraPawn->SetActorLocation(newCamLocation);
	
	float locationTolerance = 5.0f + maxEntityVelocityMag ;
	if (m_TargetLocation.Equals(oldCameraLocation, locationTolerance))
	{
		m_MovingToTarget = false;
	}
}
