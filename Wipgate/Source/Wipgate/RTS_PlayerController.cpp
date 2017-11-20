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
	ensure(m_RTS_CameraPawn != nullptr);

	TArray<UStaticMeshComponent*> meshComponents;
	m_RTS_CameraPawn->GetComponents(meshComponents);

	if (meshComponents.Num() > 0)
	{
		m_RTS_CameraPawnMeshComponent = meshComponents[0];
	}
	ensure(m_RTS_CameraPawnMeshComponent != nullptr);

	TArray<USpringArmComponent*> springArmComponents;
	m_RTS_CameraPawn->GetComponents(springArmComponents);
	if (springArmComponents.Num() > 0)
	{
		m_RTS_CameraPawnSpringArmComponent = springArmComponents[0];
	}
	ensure(m_RTS_CameraPawnSpringArmComponent != nullptr);


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
	// Update selection box size if mouse is being dragged
	if (IsInputKeyDown(EKeys::LeftMouseButton))
	{
		m_ClickEndSS = GetMousePositionVector2D();

		FVector2D selectionBoxPosition = m_ClickStartSS;
		FVector2D selectionBoxSize = (m_ClickEndSS - m_ClickStartSS);
		m_RTSHUD->UpdateSelectionBox(selectionBoxPosition, selectionBoxSize);
	}

	// If mouse is at edge of screen, update camera pos
	/* 
		These two values need to be checked because Tick can be called
		before they are set in BeginPlay somehow 
		TODO: Look into call order - I think this function even gets called in the editor for some reason
	*/
	if (m_RTS_CameraPawnMeshComponent && m_RTS_CameraPawn)
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
	else
	{
		UE_LOG(Wipgate_Log, Error, TEXT("Camera pawn or its mesh isn't set! (no biggie - just weird is all (tell AJ if you see this error))"));
	}
}

void ARTS_PlayerController::ActionMainClickPressed()
{
	m_ClickStartSS = GetMousePositionVector2D();
}

void ARTS_PlayerController::ActionMainClickReleased()
{
	// Hide selection box when mouse isn't being held
	m_RTSHUD->UpdateSelectionBox(FVector2D::ZeroVector, FVector2D::ZeroVector);

	AGameStateBase* baseGameState = GetWorld()->GetGameState();
	ARTS_GameState* castedGameState = Cast<ARTS_GameState>(baseGameState);

	// Selected units array must be cleared if shift isn't down
	if (!IsInputKeyDown(EKeys::LeftShift) && castedGameState->SelectedUnits.Num() > 0)
	{
		for (auto selectedUnit : castedGameState->SelectedUnits)
		{
			selectedUnit->SetSelected(false);
		}
		castedGameState->SelectedUnits.Empty();
	}

	for (auto unit : castedGameState->Units)
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

		Vector2DMinMax(unitBoundsMinSS, unitBoundsMaxSS);

		FVector2D selectionBoxMin = m_ClickStartSS;
		FVector2D selectionBoxMax = m_ClickEndSS;
		Vector2DMinMax(selectionBoxMin, selectionBoxMax);

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


		if (unitInSelectionBox || unitUnderCursor)
		{
			unit->SetSelected(true);
			castedGameState->SelectedUnits.AddUnique(unit);
		}
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

void ARTS_PlayerController::AxisZoom(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		float deltaArmLength = -AxisValue * GetWorld()->DeltaTimeSeconds * m_ZoomSpeed * m_FastMoveMultiplier;

		float newArmLength = m_RTS_CameraPawnSpringArmComponent->TargetArmLength + deltaArmLength;
		newArmLength = FMath::Clamp(newArmLength, m_MinArmDistance, m_MaxArmDistance);

		m_RTS_CameraPawnSpringArmComponent->TargetArmLength = newArmLength;
	}
}

void ARTS_PlayerController::AxisMoveRight(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		float camDistSpeedMultiplier = CalculateMovementSpeedBasedOnCameraZoom(GetWorld()->DeltaTimeSeconds);

		FVector rightVec = m_RTS_CameraPawnMeshComponent->GetRightVector();

		m_RTS_CameraPawn->AddActorWorldOffset(rightVec * AxisValue * camDistSpeedMultiplier * m_FastMoveMultiplier);
	}
}

void ARTS_PlayerController::AxisMoveForward(float AxisValue)
{
	if (AxisValue != 0.0f)
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

void ARTS_PlayerController::Vector2DMinMax(FVector2D& vec1, FVector2D& vec2)
{
	FVector2D vec1Copy = vec1;
	FVector2D vec2Copy = vec2;

	vec1.X = FMath::Min(vec1Copy.X, vec2Copy.X);
	vec1.Y = FMath::Min(vec1Copy.Y, vec2Copy.Y);
	vec2.X = FMath::Max(vec1Copy.X, vec2Copy.X);
	vec2.Y = FMath::Max(vec1Copy.Y, vec2Copy.Y);
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
	float movementSpeedMultiplier = m_RTS_CameraPawnSpringArmComponent->TargetArmLength * m_MoveSpeedZoomMultiplier * DeltaSeconds;
	return movementSpeedMultiplier;
}
