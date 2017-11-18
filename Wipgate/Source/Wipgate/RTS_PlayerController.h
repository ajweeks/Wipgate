// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTS_HUDBase.h"
#include "RTS_PlayerController.generated.h"

class UUserWidget;
class APawn;

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
	void AxisMouseX(float AxisValue);
	void AxisMouseY(float AxisValue);


	APawn* m_RTS_CameraPawn;
	URTS_HUDBase* m_RTSHUD;

	float m_FastMoveSpeed;
	float m_FastMoveMultiplier;
	float m_PanSensitivity;
	float m_ZoomSpeed;
	float m_MinArmDistance;
	float m_MaxArmDistance;
	float m_EdgeMoveSpeed;
	float m_EdgeSize;

	FVector2D m_ClickStartSS;
	FVector2D m_ClickEndSS;

	// TODO: Remove, not used
	FVector m_ClickStartWS;
	FVector m_ClickEndWS;
};
