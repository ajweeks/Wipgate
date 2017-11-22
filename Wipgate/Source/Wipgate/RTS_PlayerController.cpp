// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_PlayerController.h"

#include "EngineGlobals.h"
#include "Engine/Engine.h"
//#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameStateBase.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/GameFramework/SpringArmComponent.h"
#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h"
#include "Runtime/Engine/Classes/Engine/RendererSettings.h"
#include "Runtime/UMG/Public/Blueprint/WidgetLayoutLibrary.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"

#include "RTS_GameState.h"
#include "RTS_HUDBase.h"


#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text)

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
	InputComponent->BindAction("Move Fast", IE_Pressed, this, &ARTS_PlayerController::ActionMoveFastPressed);
	InputComponent->BindAction("Move Fast", IE_Released, this, &ARTS_PlayerController::ActionMoveFastReleased);
	InputComponent->BindAxis("Zoom", this, &ARTS_PlayerController::AxisZoom);
	InputComponent->BindAxis("Move Right", this, &ARTS_PlayerController::AxisMoveRight);
	InputComponent->BindAxis("Move Forward", this, &ARTS_PlayerController::AxisMoveForward);
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
	if (IsInputKeyDown(EKeys::LeftMouseButton))
	{
		m_ClickEndSS = GetMousePositionVector2D();

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
	else if (!m_DisableEdgeMovement && m_RTS_CameraPawnMeshComponent && m_RTS_CameraPawn)
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
}

void ARTS_PlayerController::ActionMainClickPressed()
{
	m_ClickStartSS = GetMousePositionVector2D();
}

void ARTS_PlayerController::ActionMainClickReleased()
{
	if (!m_RTS_GameState)
	{
		return; // Game state hasn't been initialized yet, we can't do anything
	}

	// Hide selection box when mouse isn't being held
	if (m_RTSHUD)
	{
		m_RTSHUD->UpdateSelectionBox(FVector2D::ZeroVector, FVector2D::ZeroVector);
	}

	// TODO: Use bindable key here
	const bool isShiftDown = IsInputKeyDown(EKeys::LeftShift);

	// Selected units array must be cleared if shift isn't down
	if (!isShiftDown && m_RTS_GameState->SelectedUnits.Num() > 0)
	{
		for (int i  = 0; i < m_RTS_GameState->SelectedUnits.Num(); ++i)
		{
			ARTS_UnitCharacter* selectedUnit = m_RTS_GameState->SelectedUnits[i];
			selectedUnit->SetSelected(false);
		}
		m_RTS_GameState->SelectedUnits.Empty();
	}

	for (auto unit : m_RTS_GameState->Units)
	{
		FVector unitLocation = unit->GetActorLocation();

		// Draw unit bounding box
		if (unit->ShowSelectionBox_DEBUG)
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

		bool unitInSelectionBox = 
			PointInBounds2D(unitBoundsMinSS, selectionBoxMin, selectionBoxMax) ||
			PointInBounds2D(unitBoundsMaxSS, selectionBoxMin, selectionBoxMax);


		// Check if unit is under mouse cursor (for single clicks)
		ETraceTypeQuery traceType = UEngineTypes::ConvertToTraceType(ECC_Pawn);
		FHitResult hitResult;
		bool unitUnderCursor = false;
		if (GetHitResultUnderCursorByChannel(traceType, false, hitResult))
		{
			if (hitResult.Actor == unit)
			{
				unitUnderCursor = true;
			}
		}

		const bool unitIsDead = unit->m_UnitCoreComponent->IsDead;

		const bool unitWasSelected = unit->IsSelected();
		bool unitClicked = unitUnderCursor;
		bool unitDeselected = unitClicked && isShiftDown && unitWasSelected;

		if (!unitIsDead)
		{
			if (unitDeselected)
			{
				unit->SetSelected(false);
				m_RTS_GameState->SelectedUnits.Remove(unit);
			}
			else if (unitInSelectionBox || unitClicked)
			{
				unit->SetSelected(true);
				m_RTS_GameState->SelectedUnits.AddUnique(unit);
			}
		}
	}

	if (m_RTSHUD)
	{
		m_RTSHUD->UpdateSelectedUnits(m_RTS_GameState->SelectedUnits);
	}
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

bool ARTS_PlayerController::PointInBounds2D(FVector2D point, FVector2D boundsMin, FVector2D boundsMax)
{
	bool result = ((point.X > boundsMin.X && point.X < boundsMax.X) && 
				   (point.Y > boundsMin.Y && point.Y < boundsMax.Y));
	return result;
}

void ARTS_PlayerController::FVector2DMinMax(FVector2D& vec1, FVector2D& vec2)
{
	FVector2D vec1Copy = vec1;
	FVector2D vec2Copy = vec2;

	vec1.X = FMath::Min(vec1Copy.X, vec2Copy.X);
	vec1.Y = FMath::Min(vec1Copy.Y, vec2Copy.Y);

	vec2.X = FMath::Max(vec1Copy.X, vec2Copy.X);
	vec2.Y = FMath::Max(vec1Copy.Y, vec2Copy.Y);
}

void ARTS_PlayerController::FVectorMinMax(FVector& vec1, FVector& vec2)
{
	FVector vec1Copy = vec1;
	FVector vec2Copy = vec2;

	vec1.X = FMath::Min(vec1Copy.X, vec2Copy.X);
	vec1.Y = FMath::Min(vec1Copy.Y, vec2Copy.Y);
	vec1.Z = FMath::Min(vec1Copy.Z, vec2Copy.Z);

	vec2.X = FMath::Max(vec1Copy.X, vec2Copy.X);
	vec2.Y = FMath::Max(vec1Copy.Y, vec2Copy.Y);
	vec2.Z = FMath::Max(vec1Copy.Z, vec2Copy.Z);
}

FVector2D ARTS_PlayerController::GetNormalizedMousePosition() const
{
	float mouseX, mouseY;
	GetMousePosition(mouseX, mouseY);

int32 viewportSizeX, viewportSizeY;
GetViewportSize(viewportSizeX, viewportSizeY);

FVector2D result(mouseX / (float)viewportSizeX,
	mouseY / (float)viewportSizeY);
return result;
}

FVector2D ARTS_PlayerController::GetMousePositionVector2D()
{
	FVector2D result = {};
	float viewportScaleF = UWidgetLayoutLibrary::GetMousePositionScaledByDPI(this, result.X, result.Y);
	return result;
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

		FVectorMinMax(minUnitLocation, unitLocationCopy);
		unitLocationCopy = unitLocation;
		FVectorMinMax(unitLocationCopy, maxUnitLocation);

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
