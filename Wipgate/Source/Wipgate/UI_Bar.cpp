// Fill out your copyright notice in the Description page of Project Settings.

#include "UI_Bar.h"

#include "RTS_Entity.h"

void UUI_Bar::Initialize(AActor* Owner)
{
	ARTS_Entity* castedOwner = Cast<ARTS_Entity>(Owner);
	if (castedOwner)
	{
		EntityRef = castedOwner;
	}
}

float UUI_Bar::GetHealthBarPercent()
{
	if (EntityRef && EntityRef->BaseDefenceStats.Health > 0)
	{
		float result =
			(float)(EntityRef->CurrentDefenceStats.Health) /
			(float)(EntityRef->BaseDefenceStats.Health);

		return result;
	}
	else
	{
		return 0.0f;
	}
}

FLinearColor UUI_Bar::GetTeamColor()
{
	if (EntityRef)
	{
		return EntityRef->Team.Color;
	}
	else
	{
		return FLinearColor::White;
	}
}
