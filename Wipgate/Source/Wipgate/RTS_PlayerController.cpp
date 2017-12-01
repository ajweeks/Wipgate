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
#include "Kismet/KismetSystemLibrary.h"

#include "RTS_GameState.h"
#include "RTS_HUDBase.h"
#include "Ability.h"
#include "AbilityIcon.h"
#include "GeneralFunctionLibrary_CPP.h"

DEFINE_LOG_CATEGORY(Wipgate_Log);

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
		UE_LOG(Wipgate_Log, Error, TEXT("Camera pawn doesn't contain a camera component!"));
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
		UE_LOG(Wipgate_Log, Error, TEXT("Camera pawn doesn't contain a mesh component!"));
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
		UE_LOG(Wipgate_Log, Error, TEXT("Camera pawn doesn't contain a spring arm component!"));
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
			UE_LOG(Wipgate_Log, Error, TEXT("Failed to create main HUD widget!"));
		}

		bShowMouseCursor = true;
	}
	else
	{
		UE_LOG(Wipgate_Log, Error, TEXT("Main HUD template was not set in player controller BP!"));
	}
}

void ARTS_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Main Click", IE_Pressed, this, &ARTS_PlayerController::ActionMainClickPressed);
	InputComponent->BindAction("Main Click", IE_Released, this, &ARTS_PlayerController::ActionMainClickReleased);
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
	if (IsInputKeyDown(EKeys::Escape))
	{
		// TODO: Look into alternative to this
		GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
		return;
	}

	// Update selection box size if mouse is being dragged
	if (!m_SelectedAbility && IsInputKeyDown(EKeys::LeftMouseButton))
	{
		m_ClickEndSS = UGeneralFunctionLibrary_CPP::GetMousePositionVector2D(this);

		FVector2D selectionBoxPosition = m_ClickStartSS;
		FVector2D selectionBoxSize = (m_ClickEndSS - m_ClickStartSS);

		if (m_RTSHUD)
		{
			m_RTSHUD->UpdateSelectionBox(selectionBoxPosition, selectionBoxSize);
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
			UE_LOG(Wipgate_Log, Error, TEXT("World not found!"));
		}
		else
		{
			camDistSpeedMultiplier = CalculateMovementSpeedBasedOnCameraZoom(world->DeltaTimeSeconds);
		}
	
		FVector2D normMousePos = GetNormalizedMousePosition();
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

	}

	if (m_UnitShowingAbilities)
	{
		UpdateAbilityButtons();
	}
}

void ARTS_PlayerController::ActionMainClickPressed()
{
	m_ClickStartSS = GetMousePositionVector2D();
}

void ARTS_PlayerController::ActionMainClickReleased()
{
	if (!m_RTS_GameState || !m_RTSHUD)
	{
		return; // Game state hasn't been initialized yet, we can't do anything
	}

	// Hide selection box when mouse isn't being held
	m_RTSHUD->UpdateSelectionBox(FVector2D::ZeroVector, FVector2D::ZeroVector);

	// TODO: Use bindable key here
	const bool isShiftDown = IsInputKeyDown(EKeys::LeftShift);

	if (!m_SelectedAbility)
	{
		// Selected units array must be cleared if shift isn't down
		if (!isShiftDown && m_RTS_GameState->SelectedUnits.Num() > 0)
		{
			for (int i = 0; i < m_RTS_GameState->SelectedUnits.Num(); ++i)
			{
				ARTS_UnitCharacter* selectedUnit = m_RTS_GameState->SelectedUnits[i];
				selectedUnit->SetSelected(false);
			}
			m_RTS_GameState->SelectedUnits.Empty();
		}
	}

	ETraceTypeQuery traceType = UEngineTypes::ConvertToTraceType(ECC_Pawn);
	//bool groundUnderCursor = false;
	FHitResult hitResult;
	GetHitResultUnderCursorByChannel(traceType, false, hitResult);
	//if (hitResult.bBlockingHit && hitResult.Actor.IsValid() && !hitResult.Actor.Get()->ActorHasTag("Unit"))
	//{
	//	groundUnderCursor = true; // Not 100% accurate but should work for now
	//}
	AActor* actorUnderCursor = hitResult.Actor.Get();
	ARTS_UnitCharacter* unitUnderCursor = nullptr;

	for (auto unit : m_RTS_GameState->Units)
	{
		FVector unitLocation = unit->GetActorLocation();

		// Draw unit bounding box
		if (unit->ShowSelectionBox)
		{
			UKismetSystemLibrary::DrawDebugBox(GetWorld(), unitLocation, unit->SelectionHitBox, FColor::White, FRotator::ZeroRotator, 2.0f, 4.0f);
		}

		if (unit->SelectionHitBox == FVector::ZeroVector)
		{
			UE_LOG(Wipgate_Log, Error, TEXT("Unit's selection hit box is (0, 0, 0)!"));
		}

		// Check if this unit's min or max bounding box points lie within the selection box
		FVector unitBoundsMinWS = unitLocation - unit->SelectionHitBox;
		FVector2D unitBoundsMinSS;
		ProjectWorldLocationToScreen(unitBoundsMinWS, unitBoundsMinSS, true);

		FVector unitBoundsMaxWS = unitLocation + unit->SelectionHitBox;
		FVector2D unitBoundsMaxSS;
		ProjectWorldLocationToScreen(unitBoundsMaxWS, unitBoundsMaxSS, true);

		FVector2DMinMax(unitBoundsMinSS, unitBoundsMaxSS);

		FVector2D selectionBoxMin = m_ClickStartSS;
		FVector2D selectionBoxMax = m_ClickEndSS;
		FVector2DMinMax(selectionBoxMin, selectionBoxMax);

		FVector2D viewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		const UUserInterfaceSettings* uiSettings = GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass());
		float viewportScale = uiSettings->GetDPIScaleBasedOnSize(FIntPoint((int)viewportSize.X, (int)viewportSize.Y));

		unitBoundsMinSS /= viewportScale;
		unitBoundsMaxSS /= viewportScale;

			unitInSelectionBox =
				UGeneralFunctionLibrary_CPP::PointInBounds2D(unitBoundsMinSS, selectionBoxMin, selectionBoxMax) ||
				UGeneralFunctionLibrary_CPP::PointInBounds2D(unitBoundsMaxSS, selectionBoxMin, selectionBoxMax);
		}

		// Check if unit is under mouse cursor (for single clicks)
		bool isThisUnitUnderCursor = (actorUnderCursor == unit);

		if (isThisUnitUnderCursor)
		{
			unitUnderCursor = unit;
		}

		const bool unitIsDead = unit->UnitCoreComponent->IsDead;

		const bool unitWasSelected = unit->IsSelected();
		bool unitDeselected = isThisUnitUnderCursor && isShiftDown && unitWasSelected;

		if (!m_SelectedAbility)
		{
			if (!unitIsDead)
			{
				if (unitDeselected)
				{
					if (unit == m_UnitShowingAbilities)
					{
						ClearAbilityButtons();
					}
					unit->SetSelected(false);
					m_RTS_GameState->SelectedUnits.Remove(unit);
				}
				else if (unitInSelectionBox || isThisUnitUnderCursor)
				{
					unit->SetSelected(true);
					m_RTS_GameState->SelectedUnits.AddUnique(unit);
				}
			}
		}
	}




	if (m_SelectedAbility)
	{
		EAbilityType abilityType = m_SelectedAbility->Type;
		const float abilityRange = m_SelectedAbility->CastRange;
		if (abilityRange == 0.0f && abilityType != EAbilityType::E_SELF)
		{
			UE_LOG(Wipgate_Log, Error, TEXT("Ability's cast range is 0! (this is only allowed on abilities whose type is SELF"));
		}

		switch (abilityType)
		{
		case EAbilityType::E_SELF:
		{
			UE_LOG(Wipgate_Log, Error, TEXT("Ability type is SELF but ability was selected! SELF-targeted abilities should be immediately activated."));
		} break;
		case EAbilityType::E_TARGET_GROUND:
		{
			if (hitResult.bBlockingHit)
			{
				if (!m_UnitShowingAbilities)
				{
					UE_LOG(Wipgate_Log, Error, TEXT("Unit show abilities not set before ground click!"));
				}
				else
				{
					float unitDist = FVector::DistXY(hitResult.ImpactPoint, m_UnitShowingAbilities->GetActorLocation());
					if (unitDist < abilityRange)
					{
						PrintStringToScreen(TEXT("Targetted ground"));
						m_SelectedAbility->Activate();
						m_SelectedAbility->Deselect();
						m_SelectedAbility = nullptr;
					}
					else
					{
						PrintStringToScreen(FString::Printf(TEXT("%f > %f"), unitDist, abilityRange));
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
				if (!m_UnitShowingAbilities)
				{
					UE_LOG(Wipgate_Log, Error, TEXT("Unit show abilities not set before unit click!"));
				}
				else
				{
					float unitDist = FVector::DistXY(unitUnderCursor->GetActorLocation(), m_UnitShowingAbilities->GetActorLocation());
					if (unitDist < abilityRange)
					{
						PrintStringToScreen(TEXT("Targetted unit"));
						m_SelectedAbility->SetTarget(unitUnderCursor);
						m_SelectedAbility->Activate();
						m_SelectedAbility->Deselect();
						m_SelectedAbility = nullptr;
					}
					else
					{
						PrintStringToScreen(FString::Printf(TEXT("%f > %f"), unitDist, abilityRange));
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
				EAlignment unitAlignment = unitUnderCursor->UnitCoreComponent->TeamAlignment;
				if (unitAlignment == EAlignment::E_FRIENDLY)
				{
					if (!m_UnitShowingAbilities)
					{
						UE_LOG(Wipgate_Log, Error, TEXT("Unit show abilities not set ally ground click!"));
					}
					else
					{
						float unitDist = FVector::DistXY(unitUnderCursor->GetActorLocation(), m_UnitShowingAbilities->GetActorLocation());
						if (unitDist < abilityRange)
						{
							PrintStringToScreen(TEXT("Targetted friendly"));
							m_SelectedAbility->SetTarget(unitUnderCursor);
							m_SelectedAbility->Activate();
							m_SelectedAbility->Deselect();
							m_SelectedAbility = nullptr;
						}
						else
						{
							PrintStringToScreen(FString::Printf(TEXT("%f > %f"), unitDist, abilityRange));
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
				EAlignment unitAlignment = unitUnderCursor->UnitCoreComponent->TeamAlignment;
				if (unitAlignment == EAlignment::E_ENEMY)
				{
					if (!m_UnitShowingAbilities)
					{
						UE_LOG(Wipgate_Log, Error, TEXT("Unit show abilities not set before enemy click!"));
					}
					else
					{
						float unitDist = FVector::DistXY(unitUnderCursor->GetActorLocation(), m_UnitShowingAbilities->GetActorLocation());
						if (unitDist < abilityRange)
						{
							PrintStringToScreen(TEXT("Targetted enemy"));
							m_SelectedAbility->SetTarget(unitUnderCursor);
							m_SelectedAbility->Activate();
							m_SelectedAbility->Deselect();
							m_SelectedAbility = nullptr;
						}
						else
						{
							PrintStringToScreen(FString::Printf(TEXT("%f > %f"), unitDist, abilityRange));
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



	// Ensure unit whos is showing their ability buttons is still selected
	if (m_UnitShowingAbilities)
	{
		if ((m_RTS_GameState->SelectedUnits.Num() != 1) ||
			(m_RTS_GameState->SelectedUnits[0] != m_UnitShowingAbilities))
		{
			ClearAbilityButtons();
		}
	}


	m_RTSHUD->UpdateSelectedUnits(m_RTS_GameState->SelectedUnits);

	//if (m_SelectedAbility)
	//{
	//	m_SelectedAbility->Activate();
	//	m_SelectedAbility->Deselect();
	//	m_SelectedAbility = nullptr;
	//}
	//else
	{
		if (!m_SelectedAbility && !m_UnitShowingAbilities && m_RTS_GameState->SelectedUnits.Num() == 1)
		{
			ARTS_UnitCharacter* unit = m_RTS_GameState->SelectedUnits[0];

			// TODO: Check if unit is specialist here
			bool unitIsSpecialist = true;//unit->IsSpecialist();
			if (unitIsSpecialist)
			{
				m_UnitShowingAbilities = unit;
				CreateAbilityButtons();
			}
		}
	}
}

void ARTS_PlayerController::ActionSecondaryClickPressed()
{
	if (m_SelectedAbility)
	{
		m_SelectedAbility->Deselect();
		m_SelectedAbility = nullptr;
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

	if (m_RTS_GameState->SelectedUnits.Num() == 0)
	{
		return; // No selected units to center on
	}

	m_MovingToTarget = true;
	MoveToTarget();
}
void ARTS_PlayerController::ActionSelectionGroup(TArray<ARTS_UnitCharacter*>& selectionGroupArray)
{
	for (int32 i = 0; i < m_RTS_GameState->SelectedUnits.Num(); ++i)
	{
		m_RTS_GameState->SelectedUnits[i]->SetSelected(false);
	}
	ClearAbilityButtons();

	m_RTS_GameState->SelectedUnits = selectionGroupArray;

	for (int32 i = 0; i < m_RTS_GameState->SelectedUnits.Num(); ++i)
	{
		m_RTS_GameState->SelectedUnits[i]->SetSelected(true);
	}

	if (m_RTS_GameState->SelectedUnits.Num() == 1)
	{
		ARTS_UnitCharacter* unit = m_RTS_GameState->SelectedUnits[0];
		// TODO: Check if unit is a specialist here
		bool isSpecialist = true;// unit->IsSpecialist();
		if (isSpecialist)
		{
			m_UnitShowingAbilities = unit;
			CreateAbilityButtons();
		}
	}
}

void ARTS_PlayerController::ActionSelectionGroup1()
{
	ActionSelectionGroup(m_RTS_GameState->SelectionGroup1);
}

void ARTS_PlayerController::ActionCreateSelectionGroup1()
{
	m_RTS_GameState->SelectionGroup1 = m_RTS_GameState->SelectedUnits;
}

void ARTS_PlayerController::ActionSelectionGroup2()
{
	ActionSelectionGroup(m_RTS_GameState->SelectionGroup2);
}

void ARTS_PlayerController::ActionCreateSelectionGroup2()
{
	m_RTS_GameState->SelectionGroup2 = m_RTS_GameState->SelectedUnits;
}

void ARTS_PlayerController::ActionSelectionGroup3()
{
	ActionSelectionGroup(m_RTS_GameState->SelectionGroup3);
}

void ARTS_PlayerController::ActionCreateSelectionGroup3()
{
	m_RTS_GameState->SelectionGroup3 = m_RTS_GameState->SelectedUnits;
}

void ARTS_PlayerController::ActionSelectionGroup4()
{
	ActionSelectionGroup(m_RTS_GameState->SelectionGroup4);
}

void ARTS_PlayerController::ActionCreateSelectionGroup4()
{
	m_RTS_GameState->SelectionGroup4 = m_RTS_GameState->SelectedUnits;
}

void ARTS_PlayerController::ActionSelectionGroup5()
{
	ActionSelectionGroup(m_RTS_GameState->SelectionGroup5);
}

void ARTS_PlayerController::ActionCreateSelectionGroup5()
{
	m_RTS_GameState->SelectionGroup5 = m_RTS_GameState->SelectedUnits;
}

void ARTS_PlayerController::AxisZoom(float AxisValue)
{
	if (m_RTS_CameraPawnSpringArmComponent && AxisValue != 0.0f)
	{
		float deltaArmLength = -AxisValue * GetWorld()->DeltaTimeSeconds * m_ZoomSpeed * m_FastMoveMultiplier;

		float newArmLength = m_RTS_CameraPawnSpringArmComponent->TargetArmLength + deltaArmLength;
		newArmLength = FMath::Clamp(newArmLength, m_MinArmDistance, m_MaxArmDistance);

		m_RTS_CameraPawnSpringArmComponent->TargetArmLength = newArmLength;
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
	if (m_UnitShowingAbilities)
	{
		for (int32 i = 0; i < m_UnitShowingAbilities->AbilityIcons.Num(); ++i)
		{
			m_UnitShowingAbilities->AbilityIcons[i].Button = nullptr;
			m_UnitShowingAbilities->AbilityIcons[i].ProgressBar = nullptr;
		}
		m_UnitShowingAbilities = nullptr;
		m_RTSHUD->ClearAbilityIconsFromCommandCardGrid();
	}
}

void ARTS_PlayerController::CreateAbilityButtons()
{
	if (m_UnitShowingAbilities)
	{
		for (int32 i = 0; i < m_UnitShowingAbilities->AbilityIcons.Num(); ++i)
		{
			FAbilityIcon& abilityIcon = m_UnitShowingAbilities->AbilityIcons[i];
			if (!abilityIcon.Button)
			{
				abilityIcon.ProgressBar = m_RTSHUD->ConstructWidget<UProgressBar>();
				abilityIcon.Button = m_RTSHUD->ConstructWidget<UButton>();
				int col = i;
				int row = 1;
				FLinearColor progressBarBGCol = (i == 0 ? FLinearColor::Red : (i == 1 ? FLinearColor::Blue : FLinearColor::Green));
				FLinearColor buttonBGCol = progressBarBGCol.Desaturate(0.5f);

				m_RTSHUD->AddAbilityIconToCommandCardGrid(abilityIcon.Button, abilityIcon.ProgressBar);

				m_RTSHUD->UpdateAbilityIconProperties(abilityIcon.Button, abilityIcon.ProgressBar, col, row, buttonBGCol, progressBarBGCol, m_UnitShowingAbilities);

				if (i == 0)
				{
					abilityIcon.Button->OnClicked.AddDynamic(this, &ARTS_PlayerController::OnAbilityActiveButtonPress);
				}
				else if (i == 1)
				{
					abilityIcon.Button->OnClicked.AddDynamic(this, &ARTS_PlayerController::OnAbilityConstructButtonPress);
				}
				else if (i == 2)
				{
					abilityIcon.Button->OnClicked.AddDynamic(this, &ARTS_PlayerController::OnAbilityPassiveButtonPress);
				}
			}
		}
	}
}

void ARTS_PlayerController::UpdateAbilityButtons()
{
	if (m_UnitShowingAbilities)
	{
		for (int32 i = 0; i < m_UnitShowingAbilities->AbilityIcons.Num(); ++i)
		{
			FAbilityIcon& abilityIcon = m_UnitShowingAbilities->AbilityIcons[i];
			if (abilityIcon.Button && abilityIcon.ProgressBar)
			{
				int col = i;
				int row = 1;
				FLinearColor progressBarBGCol = (i == 0 ? FLinearColor::Red : (i == 1 ? FLinearColor::Blue : FLinearColor::Green));;
				FLinearColor buttonBGCol = progressBarBGCol.Desaturate(0.5f);

				m_RTSHUD->UpdateAbilityIconProperties(abilityIcon.Button, abilityIcon.ProgressBar, col, row, buttonBGCol, progressBarBGCol, m_UnitShowingAbilities);
			}
		}
	}
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
	const int32 selectedUnitCount = m_RTS_GameState->SelectedUnits.Num();

	if (selectedUnitCount == 0)
	{
		m_MovingToTarget = false;
		return;
	}

	FVector minUnitLocation = FVector::ZeroVector;
	FVector maxUnitLocation = FVector::ZeroVector;
	FVector averageUnitLocation = FVector::ZeroVector;

	float maxUnitVelocityMag = 0.0f;

	for (int32 i = 0; i < selectedUnitCount; ++i)
	{
		ARTS_UnitCharacter* unit = m_RTS_GameState->SelectedUnits[i];
		FVector unitLocation = unit->GetActorLocation();
		averageUnitLocation += unitLocation;

		FVector unitLocationCopy = unitLocation; // TODO: Is this needed?

		UGeneralFunctionLibrary_CPP::FVectorMinMax(minUnitLocation, unitLocationCopy);
		unitLocationCopy = unitLocation;
		UGeneralFunctionLibrary_CPP::FVectorMinMax(unitLocationCopy, maxUnitLocation);

		const float unitVelocityMag = unit->GetVelocity().Size();
		if (unitVelocityMag > maxUnitVelocityMag)
		{
			maxUnitVelocityMag = unitVelocityMag;
		}
	}

	averageUnitLocation /= selectedUnitCount;

	FVector oldCameraLocation = m_RTS_CameraPawn->GetActorLocation();
	m_TargetLocation = FVector(averageUnitLocation.X, averageUnitLocation.Y, oldCameraLocation.Z);
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
	
	float locationTolerance = 5.0f + maxUnitVelocityMag ;
	if (m_TargetLocation.Equals(oldCameraLocation, locationTolerance))
	{
		m_MovingToTarget = false;
	}

	// TODO: Zoom camera in/out on selection
}
