// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RTS_Cursor.generated.h"

class UImage;
class UTexture2D;
struct FGeometry;

UCLASS()
class WIPGATE_API URTS_Cursor : public UUserWidget
{
	GENERATED_BODY()
public:
	URTS_Cursor(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void SetCursorTexture(UTexture2D* NewCursorTexture);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* DefaultTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* MoveTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* AttackMoveTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* DrugTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* Commander1Texture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* Commander2Texture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* PotionTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* SlashedCircleTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* StopCircleTexture;

};
