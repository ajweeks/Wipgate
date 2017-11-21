// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_GameState.h"

DEFINE_LOG_CATEGORY(RTS_Gamestate_log);

void ARTS_GameState::RemoveUnit(ARTS_UnitCharacter* unit)
{
	unit->SetSelected(false);
	Units.Remove(unit);
	SelectedUnits.Remove(unit);

	UE_LOG(LogTemp, Display, TEXT("ARTS_GameState::RemoveUnit > Removed unit from Units array. %i remaining."),Units.Num());
}