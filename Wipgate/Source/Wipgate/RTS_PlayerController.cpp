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
#include "UObject/ConstructorHelpers.h"

#include "Ability.h"
#include "RTS_GameState.h"
#include "RTS_HUDBase.h"
#include "RTS_Entity.h"
#include "RTS_Unit.h"
#include "RTS_Specialist.h"
#include "RTS_Team.h"
#include "AbilityIconBase.h"
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

	if (DEBUG_StartWithCurrency)
	{
		AddCurrency(1000);
		AddLuma(1000);
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

	InputComponent->BindAction("Ability Specialist Active Select", IE_Released, this, &ARTS_PlayerController::OnAbilitySpecialistActiveSelect);
	InputComponent->BindAction("Ability Specialist Construct Select", IE_Released, this, &ARTS_PlayerController::OnAbilitySpecialistConstructSelect);
	InputComponent->BindAction("Ability Specialist Passive Select", IE_Released, this, &ARTS_PlayerController::OnAbilitySpecialistPassiveSelect);

	InputComponent->BindAction("Ability Movement Move Select", IE_Released, this, &ARTS_PlayerController::OnAbilityMovementMoveSelect);
	InputComponent->BindAction("Ability Movement Attack Move Select", IE_Released, this, &ARTS_PlayerController::OnAbilityMovementAttackMoveSelect);
	InputComponent->BindAction("Ability Movement Stop Select", IE_Released, this, &ARTS_PlayerController::OnAbilityMovementStopSelect);
	InputComponent->BindAction("Ability Movement Hold Position Select", IE_Released, this, &ARTS_PlayerController::OnAbilityMovementHoldPositionSelect);

	InputComponent->BindAction("Ability Luma Apply Select", IE_Released, this, &ARTS_PlayerController::OnAbilityLumaApplySelect);

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

	InputComponent->BindAction("Invert Selection", IE_Released, this, &ARTS_PlayerController::InvertSelection);

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

void ARTS_PlayerController::UpdateSelectedEntities()
{
	UpdateSelectedEntities(m_RTS_GameState->SelectedEntities);
	m_RTSHUD->UpdateSelectedEntities(m_RTS_GameState->SelectedEntities);

	int32 currentNumEntitiesSelected = m_RTS_GameState->SelectedEntities.Num();
	if (currentNumEntitiesSelected == 0)
	{
		m_RTSHUD->HideLumaAbilityIcons();
		m_RTSHUD->HideMovementAbilityIcons();
	}
	else if (currentNumEntitiesSelected > 0)
	{
		m_RTSHUD->ShowLumaAbilityIcons();
		m_RTSHUD->ShowMovementAbilityIcons();
	}

	if (currentNumEntitiesSelected == 1)
	{
		ARTS_Entity* entity = m_RTS_GameState->SelectedEntities[0];
		ARTS_Specialist* specialist = Cast<ARTS_Specialist>(entity);
		if (specialist && specialist->CurrentDefenceStats.MaxHealth > 0)
		{
			m_SpecialistShowingAbilities = specialist;
			CreateSpecialistAbilityButtons();
		}

		m_RTSHUD->ShowSelectedEntityStats(m_RTS_GameState->SelectedEntities[0]);
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

		const bool entityIsDead = entity->CurrentDefenceStats.MaxHealth <= 0;

		const bool entityWasSelected = entity->IsSelected();
		bool entityDeselected = isThisUnitUnderCursor && isAddToSelectionKeyDown && entityWasSelected && isPrimaryClickButtonClicked;
		bool entityWasLikelyDeselectedLastFrame = isThisUnitUnderCursor && isAddToSelectionKeyDown && isPrimaryClickButtonDown && !isPrimaryClickButtonClicked && !entityWasSelected;

		if (!SelectedAbility && entity->Team->Alignment == ETeamAlignment::E_PLAYER)
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
		if (m_SpecialistShowingAbilities->CurrentDefenceStats.MaxHealth <= 0)
		{
			ClearSpecialistAbilityButtons();
			m_RTS_GameState->SelectedEntities.Empty();
			m_RTS_GameState->Entities.Remove(m_SpecialistShowingAbilities);
			m_SpecialistShowingAbilities = nullptr;
		}
		else
		{
			UpdateSpecialistAbilityButtons();
		}
	}

	if (m_RTS_GameState->SelectedEntities.Num() == 1)
	{
		m_RTSHUD->ShowSelectedEntityStats(m_RTS_GameState->SelectedEntities[0]);
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
		if (unitUnderCursor->CurrentDefenceStats.MaxHealth <= 0)
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
						if (SelectedAbility->Icon)
						{
							SelectedAbility->Icon->OnAbilityActivate();
						}
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
						if (SelectedAbility->Icon)
						{
							SelectedAbility->Icon->OnAbilityActivate();
						}
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
				ETeamAlignment entityAlignment = unitUnderCursor->Team->Alignment;
				if (entityAlignment == ETeamAlignment::E_PLAYER)
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
							if (SelectedAbility->Icon)
							{
								SelectedAbility->Icon->OnAbilityActivate();
							}
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
				ETeamAlignment entityAlignment = unitUnderCursor->Team->Alignment;
				if (entityAlignment == ETeamAlignment::E_AGGRESSIVE_AI)
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
							if (SelectedAbility->Icon)
							{
								SelectedAbility->Icon->OnAbilityActivate();
							}
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

	bool doubleClicked = false;
	if (unitUnderCursor)
	{
		float currentTimeSeconds = GetWorld()->GetTimeSeconds();

		if (unitUnderCursor->Team->Alignment == ETeamAlignment::E_PLAYER &&
			(currentTimeSeconds - m_LastEntityClickedFrameTime) <= m_DoubleClickPeriodSeconds && m_LastEntityClicked == unitUnderCursor)
		{
			doubleClicked = true;
			float targetRange = unitUnderCursor->CurrentAttackStats.Range;

			for (auto entity : m_RTS_GameState->Entities)
			{
				if (entity->Team->Alignment == ETeamAlignment::E_PLAYER && entity->CurrentAttackStats.Range == targetRange)
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
			ClearSpecialistAbilityButtons();
		}
	}

	UpdateSelectedEntities();
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
	for (int32 i = 0; i < m_RTS_GameState->SelectedEntities.Num(); ++i)
	{
		m_RTS_GameState->SelectedEntities[i]->SetSelected(false);
	}
	ClearSpecialistAbilityButtons();

	m_RTS_GameState->SelectedEntities = selectionGroupArray;

	m_RTSHUD->OnSelectionGroupSelected(Index - 1);

	for (int32 i = 0; i < m_RTS_GameState->SelectedEntities.Num(); ++i)
	{
		if (m_RTS_GameState->SelectedEntities[i]->CurrentDefenceStats.MaxHealth > 0)
		{
			m_RTS_GameState->SelectedEntities[i]->SetSelected(true);
		}
	}

	UpdateSelectedEntities();
}

void ARTS_PlayerController::ActionCreateSelectionGroup(int32 Index, TArray<ARTS_Entity*>* SelectionGroup)
{
	int32 previousSelectionEntityCount = SelectionGroup->Num();
	*SelectionGroup = m_RTS_GameState->SelectedEntities;
	if (m_RTS_GameState->SelectedEntities.Num() == 0)
	{
		if (previousSelectionEntityCount != 0)
		{
			m_RTSHUD->HideSelectionGroupIcon(Index - 1);
		}
	}
	else
	{
		m_RTSHUD->ShowSelectionGroupIcon(Index - 1);
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
			FLinearColor progressBarBGCol = (i == 0 ? FLinearColor::Red : (i == 1 ? FLinearColor::Blue : FLinearColor::Green));
			FLinearColor buttonBGCol = progressBarBGCol.Desaturate(0.5f);

			m_RTSHUD->UpdateSpecialistAbilityIconProperties(i, buttonBGCol, progressBarBGCol);
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
				FLinearColor progressBarBGCol = (i == 0 ? FLinearColor::Red : (i == 1 ? FLinearColor::Blue : FLinearColor::Green));;
				FLinearColor buttonBGCol = progressBarBGCol.Desaturate(0.5f);

				m_RTSHUD->UpdateSpecialistAbilityIconProperties(i, buttonBGCol, progressBarBGCol);
			}
		}
	}
}

void ARTS_PlayerController::InvertSelection()
{
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
			if (m_RTS_GameState->Entities[i]->Team->Alignment == ETeamAlignment::E_PLAYER &&
				m_RTS_GameState->Entities[i]->Health > 0 &&
				!m_RTS_GameState->SelectedEntities.Contains(m_RTS_GameState->Entities[i]))
			{
				newSelectedEntities.Push(m_RTS_GameState->Entities[i]);
			}
		}

		m_RTS_GameState->SelectedEntities = newSelectedEntities;

		for (int32 i = 0; i < m_RTS_GameState->SelectedEntities.Num(); ++i)
		{
			if (m_RTS_GameState->SelectedEntities[i]->Health > 0)
			{
				m_RTS_GameState->SelectedEntities[i]->SetSelected(true);
			}
		}

		if (m_RTS_GameState->SelectedEntities.Num() == 1)
		{
			ARTS_Specialist* specialst = Cast<ARTS_Specialist>(m_RTS_GameState->SelectedEntities[0]);
			if (specialst && specialst->Health > 0)
			{
				m_SpecialistShowingAbilities = specialst;
				CreateSpecialistAbilityButtons();
			}

			m_RTSHUD->ShowSelectedEntityStats(m_RTS_GameState->SelectedEntities[0]);
		}
		else
		{
			m_RTSHUD->HideSelectedEntityStats();
		}

		UpdateSelectedEntities();
	}
}

URTS_HUDBase* ARTS_PlayerController::GetRTS_HUDBase()
{
	return m_RTSHUD;
}

void ARTS_PlayerController::AddLuma(int32 LumaAmount)
{
	if (LumaAmount > 0)
	{
		m_CurrentLuma += LumaAmount;
		if (m_RTSHUD)
		{
			m_RTSHUD->UpdateLumaAmount(m_CurrentLuma);
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
}

int32 ARTS_PlayerController::GetCurrentLumaAmount()
{
	return m_CurrentLuma;
}

void ARTS_PlayerController::AddCurrency(int32 CurrencyAmount)
{
	if (CurrencyAmount > 0)
	{
		m_CurrentCurrency += CurrencyAmount;
		if (m_RTSHUD)
		{
			m_RTSHUD->UpdateCurrencyAmount(m_CurrentCurrency);
		}
	}
}

void ARTS_PlayerController::SpendCurrency(int32 CurrencyAmount)
{
	if (CurrencyAmount > 0 && m_CurrentCurrency >= CurrencyAmount)
	{
		m_CurrentCurrency -= CurrencyAmount;
		if (m_RTSHUD)
		{
			m_RTSHUD->UpdateCurrencyAmount(m_CurrentCurrency);
		}
	}
}

int32 ARTS_PlayerController::GetCurrentCurrencyAmount()
{
	return m_CurrentCurrency;
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
