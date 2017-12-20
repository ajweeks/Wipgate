// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Team.h"

void URTS_Team::AddUpgrade(FUpgrade upgrade)
{
	Upgrades.Add(upgrade);
	CalculateUpgradeEffects();
}

//TODO: function
void URTS_Team::CalculateUpgradeEffects()
{
}
