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
	// Sets default values for this character's properties
	ARTS_UnitCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintNativeEvent)
	void SetSelected(bool selected);
	virtual void SetSelected_Implementation(bool selected);

	UFUNCTION(BlueprintCallable)
	bool IsSelected() const;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	FVector SelectionHitBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	bool ShowSelectionBox_DEBUG;

	
private:
	UPROPERTY(VisibleAnywhere, Category = "Selection")
	bool Selected;
	
};
