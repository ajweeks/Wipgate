// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneralFunctionLibrary_CPP.h"

UUnitEffect * UGeneralFunctionLibrary_CPP::CreateUnitEffect(UObject * outer, const EUnitEffectStat stat, const EUnitEffectType type,
	const float delay, const int magnitude, const int duration)
{
	UUnitEffect* unitEffect = NewObject<UUnitEffect>(outer);
	unitEffect->Initialize(stat, type, delay, magnitude, duration);
	return unitEffect;
}

FString UGeneralFunctionLibrary_CPP::GetProjectVersion()
{
	FString ProjectVersion;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"),
		ProjectVersion,
		GGameIni
	);
	return ProjectVersion;
}