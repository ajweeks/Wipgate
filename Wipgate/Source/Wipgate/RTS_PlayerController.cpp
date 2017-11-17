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

ARTS_PlayerController::ARTS_PlayerController()
{
	//InputComponent->BindAction("Main Click", IE_Released, this, &ARTS_PlayerController::ActionMainClick);

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


	//CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), Main_UI_HUD);

	//CurrentWidget->AddToViewport(1000); // Render on top of everything else

	/*
	GetRootWidget();

	ConstructorHelpers::FObjectFinder<UBlueprint> BlueprintObj(TEXT("Blueprint'/Game/Path/SomeBlueprint.SomeBlueprint'"));
	*/
}

void ARTS_PlayerController::Tick(float DeltaSeconds)
{
}

void ARTS_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Main Click", IE_Pressed, this, &ARTS_PlayerController::ActionMainClick);
}

void ARTS_PlayerController::ActionMainClick()
{
	print("Main click wo ho!");
}
