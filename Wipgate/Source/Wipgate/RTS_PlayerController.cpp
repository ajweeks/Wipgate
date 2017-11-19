// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_PlayerController.h"

#include "EngineGlobals.h"
#include "Engine/Engine.h"
//#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameStateBase.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"

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

	m_RTS_CameraPawn = GetWorld()->GetFirstPlayerController()->GetPawn();

	ensure(m_RTS_CameraPawn != nullptr);

	FInputModeGameAndUI inputMode;
	inputMode.SetHideCursorDuringCapture(false);
	inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(inputMode);


	//AGameModeBase* gameMode = GetWorld()->GetAuthGameMode();


	if (MainHUD) // Check that template was set in blueprint
	{
		MainHUDInstance = CreateWidget<UUserWidget>(this, MainHUD);

		if (MainHUDInstance)
		{
			MainHUDInstance->AddToViewport();
			m_RTSHUD = Cast<URTS_HUDBase>(MainHUDInstance);
		}
		else
		{
			print("Failed to create main HUD widget!");
		}

		bShowMouseCursor = true;
	}
	else
	{
		print("Main HUD not set in player controller BP!");
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
	InputComponent->BindAxis("Mouse X", this, &ARTS_PlayerController::AxisMouseX);
	InputComponent->BindAxis("Mouse Y", this, &ARTS_PlayerController::AxisMouseY);
}

void ARTS_PlayerController::Tick(float DeltaSeconds)
{
	if (IsInputKeyDown(EKeys::LeftMouseButton))
	{
		m_ClickEndSS = GetMousePositionVector2D();

		int32 viewportSizeX, viewportSizeY;
		GetViewportSize(viewportSizeX, viewportSizeY);
		FVector2D viewportSize((float)viewportSizeX, (float)viewportSizeY);

		m_RTSHUD->SelectionBoxPosition = m_ClickStartSS;
		m_RTSHUD->SelectionBoxSize = (m_ClickEndSS - m_ClickStartSS) ;
		m_RTSHUD->UpdateSelectionBox(m_RTSHUD->SelectionBoxPosition, m_RTSHUD->SelectionBoxSize);
	}
}

void ARTS_PlayerController::ActionMainClickPressed()
{
	m_ClickStartSS = GetMousePositionVector2D();

	// TODO: Remove
	ETraceTypeQuery traceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	FHitResult hitResult;
	if (GetHitResultUnderCursorByChannel(traceType, false, hitResult))
	{
		m_ClickStartWS = hitResult.Location;
	}
}

void ARTS_PlayerController::ActionMainClickReleased()
{
	m_RTSHUD->UpdateSelectionBox(FVector2D::ZeroVector, FVector2D::ZeroVector);

	AGameStateBase* baseGameState = GetWorld()->GetGameState();
	ARTS_GameState* castedGameState = Cast<ARTS_GameState>(baseGameState);

	if (!IsInputKeyDown(EKeys::LeftShift) && castedGameState->SelectedUnits.Num() > 0)
	{
		// Selected units array must be cleared if shift isn't down
		for (auto selectedUnit : castedGameState->SelectedUnits)
		{
			selectedUnit->SetSelected(false);
		}
		castedGameState->SelectedUnits.Empty();
		UE_LOG(Wipgate_Log, Log, TEXT("Cleared selected units array"));
	}

	for (auto unit : castedGameState->Units)
	{
		FTransform unitTransform = unit->GetTransform();

		// Draw unit bounding box
		if (unit->ShowSelectionBox_DEBUG)
		{
			UKismetSystemLibrary::DrawDebugBox(GetWorld(), unitTransform.GetLocation(), unit->SelectionHitBox, FColor::White, FRotator::ZeroRotator, 2.0f, 4.0f);
		}

		// Check if selection bounding box surrounds either min or max unit bounding box points
		FVector unitBoundsMinWS = unitTransform.GetLocation() - unit->SelectionHitBox;
		FVector unitBoundsMaxWS = unitTransform.GetLocation() + unit->SelectionHitBox;
		
		FVector2D unitBoundsMinSS;
		ProjectWorldLocationToScreen(unitBoundsMinWS, unitBoundsMinSS, true);

		FVector2D unitBoundsMaxSS;
		ProjectWorldLocationToScreen(unitBoundsMaxWS, unitBoundsMaxSS, true);

		Vector2DMinMax(unitBoundsMinSS, unitBoundsMaxSS);

		FVector2D selectionBoxMin = m_ClickStartSS;
		FVector2D selectionBoxMax = m_ClickEndSS;
		Vector2DMinMax(selectionBoxMin, selectionBoxMax);

		if (PointInBounds2D(unitBoundsMinSS, selectionBoxMin, selectionBoxMax) ||
			PointInBounds2D(unitBoundsMaxSS, selectionBoxMin, selectionBoxMax))
		{
			unit->SetSelected(true);
			castedGameState->SelectedUnits.AddUnique(unit);
		}
	}


}

void ARTS_PlayerController::ActionMoveFastPressed()
{
	UE_LOG(Wipgate_Log, Log, TEXT("fast click pressed"));
}

void ARTS_PlayerController::ActionMoveFastReleased()
{
	UE_LOG(Wipgate_Log, Log, TEXT("fast click released"));
}

void ARTS_PlayerController::AxisZoom(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		UE_LOG(Wipgate_Log, Log, TEXT("Zoom: %f"), AxisValue);
	}
}

void ARTS_PlayerController::AxisMoveRight(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		UE_LOG(Wipgate_Log, Log, TEXT("Move right: %f"), AxisValue);
	}
}

void ARTS_PlayerController::AxisMoveForward(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		UE_LOG(Wipgate_Log, Log, TEXT("Move forward: %f"), AxisValue);
	}
}

void ARTS_PlayerController::AxisMouseX(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		UE_LOG(Wipgate_Log, Log, TEXT("Mouse X: %f"), AxisValue);
	}
}

void ARTS_PlayerController::AxisMouseY(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		UE_LOG(Wipgate_Log, Log, TEXT("Mouse Y: %f"), AxisValue);
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

FVector2D ARTS_PlayerController::GetMousePositionVector2D() const
{
	float mouseX, mouseY;
	GetMousePosition(mouseX, mouseY);

	FVector2D result(mouseX, mouseY);
	return result;
}
