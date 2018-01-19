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
class URTS_Squad;
class URTS_Team;
class ARTS_LevelBounds;
class ATriggerBox;
class URTS_Cursor;

UCLASS()
class WIPGATE_API ARTS_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARTS_PlayerController();

	void Initialize();
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable)
		bool IsEdgeMovementEnabled() const;

	UFUNCTION(BlueprintCallable)
		void SetEdgeMovementEnabled(bool enabled);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateSelectedEntities(const TArray<ARTS_Entity*>& SelectedEntities);

	UFUNCTION(BlueprintCallable)
		void UpdateSelectedEntitiesBase();

	// Abillity icon button press callbacks
	// Specialst ability callbacks
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilitySpecialistActiveSelect();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilitySpecialistConstructSelect();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilitySpecialistPassiveSelect();

	// Movement ability callbacks
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilityMovementMoveSelect();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilityMovementAttackMoveSelect();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilityMovementStopSelect();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilityMovementHoldPositionSelect();

	// Luma ability callbacks
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilityLumaApplySelect();


	// Specialst ability callbacks
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilitySpecialistActiveActivate();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilitySpecialistConstructActivate();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilitySpecialistPassiveActivate();

	// Movement ability callbacks
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilityMovementMoveActivate();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilityMovementAttackMoveActivate();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilityMovementStopActivate();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilityMovementHoldPositionActivate();

	// Luma ability callbacks
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnAbilityLumaApplyActivate();



	UFUNCTION(BlueprintCallable)
		void UpdateSpecialistAbilityButtons(ARTS_Specialist* SpecialistShowingAbilities = nullptr);

	UFUNCTION(BlueprintCallable)
		URTS_HUDBase* GetRTS_HUDBase();

	UFUNCTION(BlueprintCallable)
		void AddLuma(int32 LumaAmount, bool applyToEndScore = false);

	UFUNCTION(BlueprintCallable)
		void SpendLuma(int32 LumaAmount);

	UFUNCTION(BlueprintCallable)
		int32 GetCurrentLumaAmount();

	UFUNCTION(BlueprintCallable)
		void AddHealth(int32 healthAmount);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<UUserWidget> MainHUD;


	UPROPERTY(BlueprintReadWrite)
		URTS_Cursor* CursorRef;


	UPROPERTY(BlueprintReadWrite)
		AAbility* SelectedAbility = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Movement")
		TSubclassOf<AAbility> AbilityMovementMoveClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Movement")
		TSubclassOf<AAbility> AbilityMovementAttackMoveClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Movement")
		TSubclassOf<AAbility> AbilityMovementStopClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Movement")
		TSubclassOf<AAbility> AbilityMovementHoldPositionClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Luma")
		TSubclassOf<AAbility> AbilityLumaApplyClass;

	UPROPERTY(BlueprintReadWrite)
		AAbility* AbilityMovementMove;
	UPROPERTY(BlueprintReadWrite)
		AAbility* AbilityMovementAttackMove;
	UPROPERTY(BlueprintReadWrite)
		AAbility* AbilityMovementStop;
	UPROPERTY(BlueprintReadWrite)
		AAbility* AbilityMovementHoldPosition;

	UPROPERTY(BlueprintReadWrite)
		AAbility* AbilityLumaApply;

	//Team
	UPROPERTY(BlueprintReadWrite)
		URTS_Team* Team;

	// When true, camera stays centered on friendly units, regardless of selection
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
		bool AlwaysCenterOnUnits = false;

private:
	void ActionPrimaryClickPressed();
	void ActionPrimaryClickReleased();
	void ActionSecondaryClickPressed();
	void ActionSecondaryClickReleased();
	void ActionTertiaryClickPressed();
	void ActionTertiaryClickReleased();
	void ActionMoveFastPressed();
	void ActionMoveFastReleased();
	void ActionCenterOnSelectionPressed();
	void ActionCenterOnSelectionReleased();
	void ActionAddToSelectionPressed();
	void ActionAddToSelectionReleased();

public:
	// Helper function for selecting a selection group (index is 0-based)
	UFUNCTION(BlueprintCallable)
		void ActionSelectionGroup(int32 Index);

private:
	static const int32 SELECTION_GROUP_COUNT = 5;

	void ActionSelectionGroup(int32 Index, TArray<ARTS_Entity*>& selectionGroupArray);
	void ActionCreateSelectionGroup(int32 Index, TArray<ARTS_Entity*>* SelectionGroup);
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

	void ClearSpecialistAbilityButtons();
	void CreateSpecialistAbilityButtons();

	void InvertSelection();

private:

	float CalculateMovementSpeedBasedOnCameraZoom();

	void MoveToCenterOfUnits(bool FocusOnSelectedUnits);
	void MoveToTarget();

	void StartMovingToLevelEnd();
	void StartMovingToLevelStart();

	FVector ClampDCamPosWithBounds(FVector dCamPos);
	FVector ClampCamPosWithBounds(FVector camPos);

	bool IsCursorOverPurchasableItem();

public:
	UPROPERTY(BlueprintReadWrite)
		APawn* m_RTS_CameraPawn = nullptr;

private:

	UCameraComponent* m_RTS_CameraPawnCameraComponent = nullptr;
	UStaticMeshComponent* m_RTS_CameraPawnMeshComponent = nullptr;
	USpringArmComponent* m_RTS_CameraPawnSpringArmComponent = nullptr;

	UUserWidget* MainHUDInstance;

	URTS_HUDBase* m_RTSHUD = nullptr;
	ARTS_GameState* m_RTS_GameState = nullptr;

	ARTS_Specialist* m_SpecialistShowingAbilities = nullptr;

	bool m_EdgeMovementEnabled = true;

	FQuat m_StartingRotation;



	// How much faster to move when move fast key is held (shift)
	UPROPERTY(EditAnywhere, Category = "Movement")
		float m_FastMoveSpeed = 3.0f;
	// Equals Fast Move Speed when shift is down, otherwise 1.0f
		float m_FastMoveMultiplier = 1.0f;
	// The closer to zero this value is, the smaller the difference between movement speed while zoomed out and zoomed in (make denominator larger to make difference smaller)
	UPROPERTY(EditAnywhere, Category = "Movement")
		float m_MoveSpeedZoomMultiplier = 0.2f;

	// How quickly to zoom in/out when scrolling
	UPROPERTY(EditAnywhere, Category = "Movement")
		float m_ZoomSpeed = 25.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	// How far to zoom per mouse wheel turn
		float m_ZoomDistance = 140.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
		float m_MinArmDistance = 100.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
		float m_MaxArmDistance = 5000.0f;

	// How quickly to move when the mouse is at the edge of the screen
	UPROPERTY(EditAnywhere, meta = (UIMin = "1.0", UIMax = "20.0"), Category = "Movement")
		float m_EdgeMoveSpeed = 8.0f;
	// Percentage of screen from outer edges that mouse cursor will cause movement in
	UPROPERTY(EditAnywhere, meta = (UIMin = "0.001", UIMax = "0.05"), Category = "Movement")
		float m_EdgeSize = 0.005f;

	// Higher values = faster selection centering
	UPROPERTY(EditAnywhere, meta = (UIMin = "10.0", UIMax = "50.0"), Category = "Movement")
		float m_SelectionCenterMaxMoveSpeed = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
		bool m_MoveToLevelEndAtStartup = true;

	UPROPERTY(EditAnywhere)
		int m_StartingLumaAmount = 1000;

	bool m_ReturnedToStartAfterViewingEnd = false;

	FVector m_LevelStartLocation;
	FVector m_LevelEndLocation;

	bool m_MovingToSelectionCenter = false;
	float m_MovingToSelectionCenterThreshold = 5.0f;
	bool m_MovingToTarget = false;
	float m_MoveToTargetSpeed = 0.09f;
	FVector m_TargetLocation;

	float m_DelayBeforeMovingToLevelEnd = 1.2f;
	float m_DelayBeforeMovingBackToLevelStart = 2.0f;

	bool m_ZoomingToTarget = false; // True when we are zooming in but haven't yet reached our target zoom
	float m_TargetZoomArmLength;

	ARTS_LevelBounds* m_LevelBounds;

	// These two fields are private so that the setters must be used, which will update the HUD with the new values
	int32 m_CurrentLuma = 0;

	// This variable is set shift applying an ability
	// Once shift is released, this ability is deselected
	AAbility* m_AbilityOnShift = nullptr;

	FVector2D m_ClickStartSS;
	FVector2D m_ClickEndSS;

	// How many seconds to delay edge movement enabling
	// Without this, the camera can end up on the other side 
	// of the map because tick gets called while things are loading
	// but nothing gets rendered
	float m_EdgeModeDisableDelaySec = 1.5f;

	float m_LastEntityClickedFrameTime = 0.0f;
	float m_DoubleClickPeriodSeconds = 0.5f;
	ARTS_Entity* m_LastEntityClicked = nullptr;

	/* Middle mouse button panning */
	// True when MMB is down and AlwaysCenterOnUnits is false
	bool m_Panning = false;

	// Normalized screen-space mouse starting location ([0,1], [0,1])
	FVector2D m_PanMouseStartLocationSSNorm;

	// Camera's world-space location on pan start
	FVector m_PanCamStartLocation;

	// How fast middle mouse dragging pans
	UPROPERTY(EditAnywhere, Category="Movement")
		float m_PanSpeed = 12.0f;

	// How far to shift the center up (because of HUD at bottom of screen)
	UPROPERTY(EditAnywhere, Category = "Movement")
		float m_CenterOffsetY = 0.1f;

	// How far to shift the center in direction of average entity facing direction
	UPROPERTY(EditAnywhere, Category = "Movement")
		float m_CenterOffsetEntityDirection = 0.25f;

	// How fast to shift the center in direction of average entity facing direction (0=never, 1=instantly)
	UPROPERTY(EditAnywhere, Category = "Movement")
		float m_CenterOffsetEntitySpeed = 0.015f;


	TArray<URTS_Squad*> m_Squads;
};
