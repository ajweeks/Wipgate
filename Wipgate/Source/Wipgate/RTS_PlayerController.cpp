// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_PlayerController.h"

// Included for GEngine:
#include "EngineGlobals.h"
#include "Engine/Engine.h"

#include "GameFramework/GameModeBase.h"

#include "Blueprint/UserWidget.h"

//#include "GameFramework/Pawn.h"
//
//#include "Blueprint/UserWidget.h"
//
//#include "Runtime/UMG/Public/UMG.h"
//#include "Slate.h"

//#include "Runtime/Engine/Classes/Components/InputComponent.h"

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


	AGameModeBase* gameMode = GetWorld()->GetAuthGameMode();


	if (MainHUD) // Check that template was set in blueprint
	{
		MainHUDInstance = CreateWidget<UUserWidget>(this, MainHUD);

		if (MainHUDInstance)
		{
			MainHUDInstance->AddToViewport();
		}
		else
		{
			print("Failed to create main HUD widget!");
		}

		bShowMouseCursor = true;
	}
	else
	{
		//UE_LOG(LogPlayerController, Log, TEXT("Main HUD not set in game mode blueprint!"));
		print("Main HUD not set in game mode blueprint!");
	}
}

void ARTS_PlayerController::Tick(float DeltaSeconds)
{
}

void ARTS_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Main Click", IE_Pressed, this, &ARTS_PlayerController::ActionMainClick);
	InputComponent->BindAction("Move Fast", IE_Pressed, this, &ARTS_PlayerController::ActionMoveFast);
	InputComponent->BindAxis("Zoom", this, &ARTS_PlayerController::AxisZoom);
	InputComponent->BindAxis("Move Right", this, &ARTS_PlayerController::AxisMoveRight);
	InputComponent->BindAxis("Move Forward", this, &ARTS_PlayerController::AxisMoveForward);
	InputComponent->BindAxis("Mouse X", this, &ARTS_PlayerController::AxisMouseX);
	InputComponent->BindAxis("Mouse Y", this, &ARTS_PlayerController::AxisMouseY);
}

void ARTS_PlayerController::ActionMainClick()
{
	UE_LOG(Wipgate_Log, Log, TEXT("Main click"));
}

void ARTS_PlayerController::ActionMoveFast()
{
	UE_LOG(Wipgate_Log, Log, TEXT("Move fast"));
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
