// Fill out your copyright notice in the Description page of Project Settings.

#include "UI_Bar.h"

#include "RTS_Entity.h"

DEFINE_LOG_CATEGORY_STATIC(UI_BAR_LOG, Log, All);

void UUI_Bar::Initialize(AActor* Owner)
{
	Frozen = false;

	ARTS_Entity* castedOwner = Cast<ARTS_Entity>(Owner);
	if (castedOwner)
	{
		EntityRef = castedOwner;
		m_FrozenColor = EntityRef->Team.Color.Desaturate(m_FrozenColorDesaturationAmount);
	}
	else
	{
		m_FrozenColor = FLinearColor::White;
		UE_LOG(UI_BAR_LOG, Error, TEXT("UI_Bar's owner is not of type RTS_Entity! Team color will not be set"));
	}
}

float UUI_Bar::GetHealthBarPercent()
{
	if (Frozen)
	{
		return m_LastHealthBarPercent;
	}

	float result = 0.0f;
	if (EntityRef && EntityRef->BaseDefenceStats.Health > 0)
	{
		result = (float)(EntityRef->CurrentDefenceStats.Health) / (float)(EntityRef->BaseDefenceStats.Health);
	}
	
	m_LastHealthBarPercent = result;
	return result;
}

FLinearColor UUI_Bar::GetTeamColor()
{
	if (Frozen)
	{
		return m_FrozenColor;
	}

	FLinearColor result = FLinearColor::White;
	if (EntityRef)
	{
		result = EntityRef->Team.Color;
	}

	return result;
}
