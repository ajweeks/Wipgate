// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Team.h"
#include "RTS_Entity.h"

DEFINE_LOG_CATEGORY(RTS_TEAM_LOG);

//TODO: Add upgrades

void URTS_Team::AddUpgrade(FUpgrade upgrade)
{
	Upgrades.Add(upgrade);
	CalculateUpgradeEffects();
}

void URTS_Team::AddUpgradesTEST(TArray<FUpgrade> upgrades)
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
	for (int32 i = 0; i < upgradeList.Num(); ++i)
	{
		auto upgrade = upgradeList[i];
		if (upgrade.Type == EUpgradeType::E_FLAT)
		{
			for (auto entity : Entities)
			{
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
					entity->CurrentAttackStats.RateOfFire += upgrade.Effect;
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

	//TODO: Percentual upgrades
	while (upgradeList.Num() > 0)
	{
		//Upgrade to compare with
		auto checkUpgrade = upgradeList[0];
		float totalEffect = 1.f;
		TArray<int32> toRemove;
		for (int32 i = 0; i < upgradeList.Num(); ++i)
		{
			auto upgrade = upgradeList[i];

			if(checkUpgrade.Type == EUpgradeType::E_PERCENTUAL)
			{
				//Check for upgrades of that type
				if (checkUpgrade.Stat == upgrade.Stat)
				{
					totalEffect += upgrade.Effect;
					toRemove.Add(i);
				}
			}
			else
			{
				UE_LOG(RTS_TEAM_LOG, Warning, TEXT("ARTS_Team::CalculateUpgradeEffects > Non-percentual upgrade found!"));
			}
		}

		//Remove upgrades
		for (int32 i = toRemove.Num() - 1; i >= 0; --i)
		{
			upgradeList.RemoveAt(toRemove[i]);
		}

		//Apply effect
		for (auto entity : Entities)
		{
			switch (checkUpgrade.Stat)
			{
			case EUpgradeStat::E_ARMOR:
				entity->CurrentDefenceStats.Armor *= totalEffect;
				UE_LOG(RTS_TEAM_LOG, Warning, TEXT("ARTS_Team::CalculateUpgradeEffects > Armor upgrade: %f"), totalEffect);
				break;
			case EUpgradeStat::E_DAMAGE:
				entity->CurrentAttackStats.Damage *= totalEffect;
				UE_LOG(RTS_TEAM_LOG, Warning, TEXT("ARTS_Team::CalculateUpgradeEffects > Damage upgrade: %f"), totalEffect);
				break;
			case EUpgradeStat::E_HEALTH:
				entity->CurrentDefenceStats.MaxHealth *= totalEffect;
				UE_LOG(RTS_TEAM_LOG, Warning, TEXT("ARTS_Team::CalculateUpgradeEffects > Health upgrade: %f"), totalEffect);
				break;
			case EUpgradeStat::E_RANGE:
				entity->CurrentAttackStats.Range *= totalEffect;
				UE_LOG(RTS_TEAM_LOG, Warning, TEXT("ARTS_Team::CalculateUpgradeEffects > Range upgrade: %f"), totalEffect);
				break;
			case EUpgradeStat::E_RATEOFFIRE:
				entity->CurrentAttackStats.RateOfFire *= totalEffect;
				UE_LOG(RTS_TEAM_LOG, Warning, TEXT("ARTS_Team::CalculateUpgradeEffects > Rate of Fire upgrade: %f"), totalEffect);
				break;
			case EUpgradeStat::E_SPEED:
				entity->CurrentMovementStats.Speed *= totalEffect;
				UE_LOG(RTS_TEAM_LOG, Warning, TEXT("ARTS_Team::CalculateUpgradeEffects > Speed upgrade: %f"), totalEffect);
				break;
			case EUpgradeStat::E_VISION:
				entity->CurrentVisionStats.InnerRange *= totalEffect;
				entity->CurrentVisionStats.OuterRange *= totalEffect;
				UE_LOG(RTS_TEAM_LOG, Warning, TEXT("ARTS_Team::CalculateUpgradeEffects > Vision upgrade: %f"), totalEffect);
				break;
			default:
				break;
			}
		}
	}

}
