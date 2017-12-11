// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Specialist.h"
#include "Ability.h"

DEFINE_LOG_CATEGORY_STATIC(RTS_SPECIALIST_LOG, Log, All);

ARTS_Specialist::ARTS_Specialist()
{
}

void ARTS_Specialist::Kill()
{
	ARTS_Unit::Kill();

	if (AbilityActive)
	{
		AbilityActive->Destroy();
	}
	if (AbilityConstruct)
	{
		AbilityConstruct->Destroy();
	}
	if (AbilityPassive)
	{
		AbilityPassive->Destroy();
	}
}
