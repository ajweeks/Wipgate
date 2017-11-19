// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTS_HUDBase.h"
#include "RTS_PlayerController.generated.h"

class UUserWidget;
class APawn;
class UStaticMeshComponent;
class USpringArmComponent;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> MainHUD;
		
	UUserWidget* MainHUDInstance;

private:
	void ActionMainClickPressed();
	void ActionMainClickReleased();
	void ActionMoveFastPressed();
	void ActionMoveFastReleased();
	void AxisZoom(float AxisValue);
	void AxisMoveForward(float AxisValue);
	void AxisMoveRight(float AxisValue);


	bool PointInBounds2D(FVector2D point, FVector2D boundsMin, FVector2D boundsMax);
	void Vector2DMinMax(FVector2D& vec1, FVector2D& vec2); // Calls Min and Max for each component
	FVector2D GetNormalizedMousePosition() const;
	FVector2D GetMousePositionVector2D();
	float CalculateMovementSpeedBasedOnCameraZoom(float DeltaSeconds);

	APawn* m_RTS_CameraPawn = nullptr;
	UStaticMeshComponent* m_RTS_CameraPawnMeshComponent = nullptr;
	USpringArmComponent* m_RTS_CameraPawnSpringArmComponent = nullptr;
	URTS_HUDBase* m_RTSHUD = nullptr;

	// TODO: Expose most of these to the user to set manually
	float m_FastMoveSpeed = 5.0f; // How much faster to move when move fast key is held (shift)
	float m_FastMoveMultiplier = 1.0f; // Equals m_FastMoveSpeed when move fast button is down, otherwise 1.0f
	float m_PanSensitivity = 200.0f;
	float m_MoveSpeedZoomMultiplier = 1.0f / 5.0f;

	float m_ZoomSpeed = 3000.0f;
	float m_MinArmDistance = 400.0f;
	float m_MaxArmDistance = 4000.0f;
	
	float m_EdgeMoveSpeed = 3.0f;
	float m_EdgeSize = 0.035f;

	FVector2D m_ClickStartSS;
	FVector2D m_ClickEndSS;
};
