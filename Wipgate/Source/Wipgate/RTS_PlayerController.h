// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTS_HUDBase.h"
#include "RTS_PlayerController.generated.h"

class UUserWidget;
class UCameraComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class APawn;
class ARTS_GameState;
class AAbility;
class ARTS_Entity;
class ARTS_Unit;
class ARTS_Specialist;

DECLARE_LOG_CATEGORY_EXTERN(Wipgate_Log, Log, All);

UCLASS()
class WIPGATE_API ARTS_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARTS_PlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable)
	bool IsEdgeMovementEnabled() const;

	UFUNCTION(BlueprintCallable)
	void SetEdgeMovementEnabled(bool enabled);

	UFUNCTION(BlueprintImplementableEvent)
	void OnAbilityActiveButtonPress();

	UFUNCTION(BlueprintImplementableEvent)
	void OnAbilityConstructButtonPress();

	UFUNCTION(BlueprintImplementableEvent)
	void OnAbilityPassiveButtonPress();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> MainHUD;

	UUserWidget* MainHUDInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AAbility* m_SelectedAbility = nullptr;

	UFUNCTION(BlueprintCallable)
	void UpdateAbilityButtons(ARTS_UnitCharacter* SpecialistShowingAbilities = nullptr);

private:
	void ActionMainClickPressed();
	void ActionMainClickReleased();
	void ActionSecondaryClickPressed();
	void ActionSecondaryClickReleased();
	void ActionMoveFastPressed();
	void ActionMoveFastReleased();
	void ActionCenterOnSelection();

	void ActionSelectionGroup(TArray<ARTS_Entity*>& selectionGroupArray);
	void ActionSelectionGroup1();
	void ActionCreateSelectionGroup1();
	void ActionSelectionGroup2();
	void ActionCreateSelectionGroup2();
	void ActionSelectionGroup3();
	void ActionCreateSelectionGroup3();
	void ActionSelectionGroup4();
	void ActionCreateSelectionGroup4();
	void ActionSelectionGroup5();
	void ActionCreateSelectionGroup5();

	void AxisZoom(float AxisValue);
	void AxisMoveForward(float AxisValue);
	void AxisMoveRight(float AxisValue);

	void ClearAbilityButtons();
	void CreateAbilityButtons();

private:

	float CalculateMovementSpeedBasedOnCameraZoom(float DeltaSeconds);

	void MoveToTarget();

	APawn* m_RTS_CameraPawn = nullptr;
	UCameraComponent* m_RTS_CameraPawnCameraComponent = nullptr;
	UStaticMeshComponent* m_RTS_CameraPawnMeshComponent = nullptr;
	USpringArmComponent* m_RTS_CameraPawnSpringArmComponent = nullptr;
	URTS_HUDBase* m_RTSHUD = nullptr;
	ARTS_GameState* m_RTS_GameState = nullptr;

	ARTS_Specialist* m_SpecialistShowingAbilities = nullptr;

	UPROPERTY(EditAnywhere, Category = "Misc")
	bool m_EdgeMovementEnabled = true;

	// TODO: Store the value for this in each map
	UPROPERTY(EditAnywhere, Category = "Misc")
	FVector m_StartingLocation;
	// TODO: Store the value for this in each map
	UPROPERTY(EditAnywhere, Category = "Misc")
	FQuat m_StartingRotation;

	// How much faster to move when move fast key is held (shift)
	UPROPERTY(EditAnywhere, Category = "Movement")
	float m_FastMoveSpeed = 5.0f;
	// Equals Fast Move Speed when shift is down, otherwise 1.0f
	float m_FastMoveMultiplier = 1.0f;
	// Higer values = faster movement when more zoomed out
	UPROPERTY(EditAnywhere, Category = "Movement")
	float m_MoveSpeedZoomMultiplier = 1.0f / 5.0f;

	// How quickly to zoom in/out when scrolling
	UPROPERTY(EditAnywhere, Category = "Movement")
	float m_ZoomSpeed = 6000.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float m_MinArmDistance = 100.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float m_MaxArmDistance = 5000.0f;

	// How quickly to move when the mouse is at the edge of the screen
	UPROPERTY(EditAnywhere, meta = (UIMin = "1.0", UIMax = "20.0"), Category = "Movement")
	float m_EdgeMoveSpeed = 10.0f;
	// Percentage of screen from outer edges that mouse cursor will cause movement in
	UPROPERTY(EditAnywhere, meta = (UIMin = "0.001", UIMax = "0.05"), Category = "Movement")
	float m_EdgeSize = 0.005f;

	// Higher values = faster selection centering
	UPROPERTY(EditAnywhere, meta = (UIMin = "10.0", UIMax = "50.0"), Category = "Movement")
	float m_SelectionCenterMaxMoveSpeed = 25.0f;

	bool m_MovingToTarget = false; // True when we are taking several frames to move to a target location
	FVector m_TargetLocation;

	FVector2D m_ClickStartSS;
	FVector2D m_ClickEndSS;

	// TODO: Remove, not used
	FVector m_ClickStartWS;
	FVector m_ClickEndWS;
};
