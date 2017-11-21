// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RTS_UnitCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(RTS_Unit_Log, Log, All);

UCLASS()
class WIPGATE_API ARTS_UnitCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARTS_UnitCharacter();

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintNativeEvent)
	void SetSelected(bool selected);
	virtual void SetSelected_Implementation(bool selected);

	UFUNCTION(BlueprintGetter)
	bool IsSelected() const;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	FVector SelectionHitBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	bool ShowSelectionBox_DEBUG;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Selection")
	bool Selected;
	
};
