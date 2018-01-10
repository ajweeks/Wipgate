// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Cursor.h"
#include "Layout/Geometry.h"
#include "Image.h"
#include "Engine/Texture2D.h"
#include "RTS_PlayerController.h"
#include "ConstructorHelpers.h"
#include "GeneralFunctionLibrary_CPP.h"

DEFINE_LOG_CATEGORY_STATIC(RTS_Cursor_Log, Log, All);

URTS_Cursor::URTS_Cursor(const FObjectInitializer& ObjectInitializer) :
	UUserWidget(ObjectInitializer)
{
	AttemptToFindObjectByPath(&DefaultTexture, TEXT("Texture2D'/Game/Art/Cursors/default-cursor.default-cursor'"));
	AttemptToFindObjectByPath(&MoveTexture, TEXT("Texture2D'/Game/Art/Cursors/MoveCursor.MoveCursor'"));
	AttemptToFindObjectByPath(&AttackMoveTexture, TEXT("Texture2D'/Game/Art/Cursors/AttackCursor.AttackCursor'"));
	AttemptToFindObjectByPath(&DrugTexture, TEXT("Texture2D'/Game/Art/Cursors/DrugCursor.DrugCursor'"));
	AttemptToFindObjectByPath(&Commander1Texture, TEXT("Texture2D'/Game/Art/Cursors/CommanderCursor.CommanderCursor'"));
	AttemptToFindObjectByPath(&Commander2Texture, TEXT("Texture2D'/Game/Art/Cursors/CommanderCursor2.CommanderCursor2'"));
	AttemptToFindObjectByPath(&PotionTexture, TEXT("Texture2D'/Game/Art/Cursors/potioncursor.potioncursor'"));
	AttemptToFindObjectByPath(&SlashedCircleTexture, TEXT("Texture2D'/Game/Art/Cursors/slashed-circle-cursor.slashed-circle-cursor'"));
	AttemptToFindObjectByPath(&StopCircleTexture, TEXT("Texture2D'/Game/Art/Cursors/stop-cursor.stop-cursor'"));
}

