// Fill out your copyright notice in the Description page of Project Settings.

#include "UI_Bar.h"
#include "RTS_Team.h"
#include "RTS_Entity.h"

DEFINE_LOG_CATEGORY_STATIC(UI_BAR_LOG, Log, All);

void UUI_Bar::InitializeFromOwner(AActor* Owner)
{
	Frozen = false;

	ARTS_Entity* castedOwner = Cast<ARTS_Entity>(Owner);
	if (castedOwner)
	{
		EntityRef = castedOwner;
		if(EntityRef->Team)
			m_FrozenHealthBarColor = EntityRef->Team->Color.Desaturate(m_FrozenColorDesaturationAmount);
	}
	else
	{
		m_FrozenHealthBarColor = FLinearColor::White;
		UE_LOG(UI_BAR_LOG, Error, TEXT("UI_Bar's owner is not of type RTS_Entity! Team color will not be set"));
	}

	m_LumaBarColor = FLinearColor(0.173f, 1.0f, 1.0f);
	m_FrozenLumaBarColor = m_LumaBarColor.Desaturate(m_FrozenColorDesaturationAmount);
}

float UUI_Bar::GetHealthBarPercent()
{
	if (Frozen)
	{
		return m_LastHealthBarPercent;
	}

	float percent = 0.0f;
	if (EntityRef && EntityRef->BaseDefenceStats.MaxHealth > 0)
	{
		percent = (float)(EntityRef->Health) / (float)(EntityRef->BaseDefenceStats.MaxHealth);
	}

	m_LastHealthBarPercent = percent;
	return percent;
}

float UUI_Bar::GetLumaBarPercent()
{
	if (Frozen)
	{
		return m_LastLumaBarPercent;
	}

	float percent = 0.0f;
	if (EntityRef && EntityRef->BaseDefenceStats.MaxHealth > 0)
	{
		percent = (float)(EntityRef->CurrentLumaStats.LumaSaturation) / (float)(EntityRef->CurrentLumaStats.MaxLumaSaturation);
	}

	m_LastLumaBarPercent = percent;
	return percent;
}

FLinearColor UUI_Bar::GetHealthBarColor()
{
	if (Frozen)
	{
		return m_FrozenHealthBarColor;
	}

	FLinearColor result = FLinearColor::White;
	if (EntityRef && EntityRef->Team)
	{
		result = EntityRef->Team->Color;
	}

	return result;
}

FLinearColor UUI_Bar::GetLumaBarColor()
{
	if (Frozen)
	{
		return m_FrozenLumaBarColor;
	}

	return m_LumaBarColor;
}
