// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_GameInstance.h"

void URTS_GameInstance::AddLuma(int amount)
{
	CurrentLuma += amount;
}

void URTS_GameInstance::SpendLuma(int amount)
{
	CurrentLuma -= amount;
}
