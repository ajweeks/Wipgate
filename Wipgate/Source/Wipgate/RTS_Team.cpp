// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Team.h"
#include "RTS_Entity.h"

DEFINE_LOG_CATEGORY(RTS_TEAM_LOG);

void URTS_Team::AddUpgrade(FUpgrade upgrade)
{
	Upgrades.Add(upgrade);
	CalculateUpgradeEffects();
}

void URTS_Team::AddUpgrades_CPP(TArray<FUpgrade> upgrades)
{
	for (auto upgrade : upgrades)
	{
		Upgrades.Add(upgrade);
	}
	CalculateUpgradeEffects();
}

void URTS_Team::CalculateUpgradeEffects()
{
	auto upgradeList = Upgrades;

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
	for (int32 i = upgradeList.Num() - 1; i >= 0 ; --i)
	{
		auto upgrade = upgradeList[i];
		if (upgrade.Type == EUpgradeType::E_FLAT)
		{
			for (auto entity : Entities)
			{
				if (upgrade.AffectedType != entity->EntityType)
					continue;

				//Apply upgrade
				switch (upgrade.Stat)
				{
				case EUpgradeStat::E_ARMOR:
					entity->CurrentDefenceStats.Armor += upgrade.Effect;
					break;
				case EUpgradeStat::E_DAMAGE:
					entity->CurrentAttackStats.Damage += upgrade.Effect;
					break;
				case EUpgradeStat::E_HEALTH:
					entity->CurrentDefenceStats.MaxHealth += upgrade.Effect;
					break;
				case EUpgradeStat::E_RANGE:
					entity->CurrentAttackStats.Range += upgrade.Effect;
					break;
				case EUpgradeStat::E_RATEOFFIRE:
					entity->CurrentAttackStats.AttackCooldown += upgrade.Effect;
					break;
				case EUpgradeStat::E_SPEED:
					entity->CurrentMovementStats.Speed += upgrade.Effect;
					break;
				case EUpgradeStat::E_VISION:
					entity->CurrentVisionStats.InnerRange += upgrade.Effect;
					entity->CurrentVisionStats.OuterRange += upgrade.Effect;
					break;
				default:
					break;
				}
			}

			//Remove from list
			upgradeList.RemoveAt(i);
		}
	}

	while (upgradeList.Num() > 0)
	{
		//Upgrade to compare with
		auto checkUpgrade = upgradeList[upgradeList.Num()-1];
		float totalEffect = 1.f;
		//TArray<int32> toRemove;
		for (int32 i = upgradeList.Num()-1; i >= 0; --i)
		{
			auto upgrade = upgradeList[i];

			if(checkUpgrade.Type == EUpgradeType::E_PERCENTUAL)
			{
				//Check for upgrades of that type
				if (checkUpgrade.Stat == upgrade.Stat)
				{
					//Check if it applies to the unit type
					if (checkUpgrade.AffectedType == upgrade.AffectedType)
					{
						totalEffect += upgrade.Effect;
						upgradeList.RemoveAt(i);
						//break;
					}
				}
			}
			else
			{
				UE_LOG(RTS_TEAM_LOG, Warning, TEXT("ARTS_Team::CalculateUpgradeEffects > Non-percentual upgrade found!"));
			}
		}

		////Remove upgrades
		//for (int32 i = toRemove.Num() - 1; i >= 0; --i)
		//{
		//	upgradeList.RemoveAt(toRemove[i]);
		//}

		//Apply effect
		for (auto entity : Entities)
		{
			if (checkUpgrade.AffectedType != entity->EntityType)
				continue;

			switch (checkUpgrade.Stat)
			{
			case EUpgradeStat::E_ARMOR:
				entity->CurrentDefenceStats.Armor *= totalEffect;
				break;
			case EUpgradeStat::E_DAMAGE:
				entity->CurrentAttackStats.Damage *= totalEffect;
				break;
			case EUpgradeStat::E_HEALTH:
				entity->CurrentDefenceStats.MaxHealth *= totalEffect;
				break;
			case EUpgradeStat::E_RANGE:
				entity->CurrentAttackStats.Range *= totalEffect;
				break;
			case EUpgradeStat::E_RATEOFFIRE:
				entity->CurrentAttackStats.AttackCooldown *= totalEffect;
				break;
			case EUpgradeStat::E_SPEED:
				entity->CurrentMovementStats.Speed *= totalEffect;
				break;
			case EUpgradeStat::E_VISION:
				entity->CurrentVisionStats.InnerRange *= totalEffect;
				entity->CurrentVisionStats.OuterRange *= totalEffect;
				break;
			default:
				break;
			}
		}
	}

}
