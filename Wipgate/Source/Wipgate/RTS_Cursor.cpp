// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Cursor.h"
#include "Layout/Geometry.h"
#include "Image.h"
#include "Engine/Texture2D.h"
#include "RTS_PlayerController.h"
#include "GeneralFunctionLibrary_CPP.h"

DEFINE_LOG_CATEGORY_STATIC(RTS_Cursor_Log, Log, All);

URTS_Cursor::URTS_Cursor(const FObjectInitializer& ObjectInitializer) :
	UUserWidget(ObjectInitializer)
{
	//AttemptToFindObjectByPath(&DefaultTexture, TEXT("Texture2D'/Game/Art/Cursors/pointyhand.pointyhand'"));
	//AttemptToFindObjectByPath(&GrabbedTexture, TEXT("Texture2D'/Game/Art/Cursors/closedhand.closedhand'"));
	//AttemptToFindObjectByPath(&MoveTexture, TEXT("Texture2D'/Game/Art/Cursors/MoveCursor.MoveCursor'"));
	//AttemptToFindObjectByPath(&AttackMoveTexture, TEXT("Texture2D'/Game/Art/Cursors/AttackCursor2.AttackCursor2'"));
	//AttemptToFindObjectByPath(&DrugTexture, TEXT("Texture2D'/Game/Art/Cursors/DrugCursor.DrugCursor'"));
	//AttemptToFindObjectByPath(&CommanderTexture, TEXT("Texture2D'/Game/Art/Cursors/CommanderCursor.CommanderCursor'"));
	//AttemptToFindObjectByPath(&PotionTexture, TEXT("Texture2D'/Game/Art/Cursors/potioncursor.potioncursor'"));
	//AttemptToFindObjectByPath(&InvalidTexture, TEXT("Texture2D'/Game/Art/Cursors/invalid_cursor.invalid_cursor'"));
	//AttemptToFindObjectByPath(&ShopTexture, TEXT("Texture2D'/Game/Art/Cursors/shop_cursor.shop_cursor'"));
}
