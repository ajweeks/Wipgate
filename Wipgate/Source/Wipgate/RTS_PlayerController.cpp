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
}

void ARTS_PlayerController::ActionMainClickPressed()
{
	float mouseX, mouseY;
	GetMousePosition(mouseX, mouseY);

	int32 viewportSizeX, viewportSizeY;
	GetViewportSize(viewportSizeX, viewportSizeY);

	m_ClickStartSS.X = mouseX / (float)viewportSizeX;
	m_ClickStartSS.Y = mouseY / (float)viewportSizeY;

	UE_LOG(Wipgate_Log, Log, TEXT("Click start SS: (%f, %f)"), m_ClickStartSS.X, m_ClickStartSS.Y);

	ETraceTypeQuery traceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	FHitResult hitResult;
	if (GetHitResultUnderCursorByChannel(traceType, false, hitResult))
	{
		m_ClickStartWS = hitResult.Location;
		UE_LOG(Wipgate_Log, Log, TEXT("Click start WS: (%f, %f, %f)"), m_ClickStartWS.X, m_ClickStartWS.Y, m_ClickStartWS.Z);
	}
}

void ARTS_PlayerController::ActionMainClickReleased()
{
	m_RTSHUD->UpdateSelectionBox(FVector2D::ZeroVector, FVector2D::ZeroVector);

	AGameStateBase* baseGameState = GetWorld()->GetGameState();
	ARTS_GameState* castedGameState = Cast<ARTS_GameState>(baseGameState);

	if (!IsInputKeyDown(EKeys::LeftShift))
	{
		// Clear selected units array
		for (auto selectedUnit : castedGameState->SelectedUnits)
		{
			selectedUnit->SetSelected(false);
		}
		castedGameState->SelectedUnits.Empty();
	}

	for (auto unit : castedGameState->Units)
	{
		FTransform unitTransform = unit->GetTransform();
		if (unit->ShowSelectionBox_DEBUG)
		{
			UKismetSystemLibrary::DrawDebugBox(nullptr, unitTransform.GetLocation(), unit->SelectionHitBox, FColor::White, FRotator::ZeroRotator, 2.0f, 4.0f);
			UE_LOG(Wipgate_Log, Log, TEXT("Unit location: (%f, %f, %f)"), unitTransform.GetLocation().X, unitTransform.GetLocation().Y, unitTransform.GetLocation().Z);
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
