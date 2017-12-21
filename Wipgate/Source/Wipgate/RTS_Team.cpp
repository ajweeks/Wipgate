// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Team.h"
#include "RTS_Entity.h"

DEFINE_LOG_CATEGORY(RTS_TEAM_LOG);

void URTS_Team::AddUpgrade(FUpgrade upgrade)
{
	Upgrades.Add(upgrade);
	CalculateUpgradeEffects();
}

void URTS_Team::CalculateUpgradeEffects()
{
	//TODO: Check debuffs
	//Reset all values
	for (auto entity : Entities)
	{
		entity->CurrentAttackStats = entity->BaseAttackStats;
		entity->CurrentDefenceStats = entity->BaseDefenceStats;
		entity->CurrentMovementStats = entity->BaseMovementStats;
		entity->CurrentVisionStats = entity->BaseVisionStats;
	}

	//Remove all flat upgrades
	for (int32 i = 0; i < Upgrades.Num(); ++i)
	{
		auto upgrade = Upgrades[i];
		if (upgrade.Type == EUpgradeType::E_FLAT)
		{
			for (auto entity : Entities)
			{
				//TODO: Vision
				//TODO: Health
				//Apply upgrade
				switch (upgrade.Stat)
				{
				case EUpgradeStat::E_ARMOR:
					entity->CurrentDefenceStats.Armor += upgrade.Effect;
					break;
				case EUpgradeStat::E_DAMAGE:
					entity->CurrentAttackStats.Damage += upgrade.Effect;
					break;
				//case EUpgradeStat::E_HEALTH:
				//	entity->CurrentDefenceStats.Health += upgrade.Effect;
				//	break;
				case EUpgradeStat::E_RANGE:
					entity->CurrentMovementStats.Speed += upgrade.Effect;
					break;
				case EUpgradeStat::E_RATEOFFIRE:
					entity->CurrentAttackStats.RateOfFire += upgrade.Effect;
					break;
				case EUpgradeStat::E_SPEED:
					entity->CurrentMovementStats.Speed += upgrade.Effect;
					break;
				default:
					break;
				}
			}

			//Remove from list
			Upgrades.RemoveAt(i);
		}
	}

	//TODO: Percentual upgrades

}
