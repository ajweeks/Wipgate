// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "RTS_HUDBase.generated.h"

class ARTS_PlayerController;

UCLASS()
class WIPGATE_API URTS_HUDBase : public UUserWidget
{
	GENERATED_BODY()

public:

	
private:
	ARTS_PlayerController* m_PlayerController;
	SBorder m_Border;

};
