// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Team.h"
#include "RTS_Entity.h"
#include "RTS_GameInstance.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY(RTS_TEAM_LOG);

void URTS_Team::AddUpgrades(TArray<FUpgrade> upgrades)
{
	if (!World)
	{
		UE_LOG(RTS_TEAM_LOG, Warning, TEXT("AddUpgrades > No world found"));
		return;
	}
	auto gameinstance = World->GetGameInstance<URTS_GameInstance>();
	if (!gameinstance)
	{
		UE_LOG(RTS_TEAM_LOG, Warning, TEXT("AddUpgrades > No game instance found"));
		return;
	}

	for (auto upgrade : upgrades)
	{
		Upgrades.Add(upgrade);

		//Add to game instance
		gameinstance->ActiveUpgrades.Add(upgrade);
	}
	CalculateUpgradeEffects();
}

FAttackStat URTS_Team::GetUpgradedAttackStats(ARTS_Entity* entity)
{
	auto upgradeList = Upgrades;
	FAttackStat upgradedAttackStats = entity->BaseAttackStats;

	//Remove all flat upgrades
	for (int32 i = upgradeList.Num() - 1; i >= 0; --i)
	{
		auto upgrade = upgradeList[i];
		if (upgrade.Type == EUpgradeType::E_FLAT)
		{
			if (upgrade.AffectedType == entity->EntityType)
			{
				//Apply upgrade
				switch (upgrade.Stat)
				{
				case EUpgradeStat::E_DAMAGE:
					upgradedAttackStats.Damage += upgrade.Effect;
					upgradedAttackStats.Damage = FMath::Clamp(upgradedAttackStats.Damage, 0, upgradedAttackStats.Damage);
					break;
				case EUpgradeStat::E_RANGE:
					upgradedAttackStats.Range += upgrade.Effect;
					upgradedAttackStats.Range = FMath::Clamp(upgradedAttackStats.Range, 0.f, upgradedAttackStats.Range);
					break;
				case EUpgradeStat::E_RATEOFFIRE:
					UE_LOG(RTS_TEAM_LOG, Warning, TEXT("CalculateUpgradeEffects > Flat upgrades don't work on rate of fire!"))
					break;
				default:
					break;
				}
			}

			upgradeList.RemoveAt(i);
		}
	}

	while (upgradeList.Num() > 0)
	{
		//Upgrade to compare with
		auto checkUpgrade = upgradeList[upgradeList.Num() - 1];
		float totalEffect = 1.0f;
		for (int32 i = upgradeList.Num() - 1; i >= 0; --i)
		{
			auto upgrade = upgradeList[i];

			if (checkUpgrade.Type == EUpgradeType::E_PERCENTUAL)
			{
				//Check for upgrades of that type
				if (checkUpgrade.Stat == upgrade.Stat)
				{
					//Check if it applies to the unit type
					if (checkUpgrade.AffectedType == upgrade.AffectedType)
					{
						totalEffect += upgrade.Effect;
						upgradeList.RemoveAt(i);
					}
				}
			}
			else
			{
				UE_LOG(RTS_TEAM_LOG, Warning, TEXT("ARTS_Team::GetUpgradedAttackStats > Non-percentual upgrade found!"));
			}
		}

		if (checkUpgrade.AffectedType != entity->EntityType)
			continue;

		switch (checkUpgrade.Stat)
		{
		case EUpgradeStat::E_DAMAGE:
			upgradedAttackStats.Damage *= totalEffect;
			upgradedAttackStats.Damage = FMath::Clamp(upgradedAttackStats.Damage, 0, upgradedAttackStats.Damage);
			break;
		case EUpgradeStat::E_RANGE:
			upgradedAttackStats.Range *= totalEffect;
			upgradedAttackStats.Range = FMath::Clamp(upgradedAttackStats.Range, 0.f, upgradedAttackStats.Range);
			break;
		case EUpgradeStat::E_RATEOFFIRE:
			upgradedAttackStats.AttackCooldown *= totalEffect;
			upgradedAttackStats.AttackCooldown = FMath::Clamp(upgradedAttackStats.AttackCooldown, 0.f, upgradedAttackStats.AttackCooldown);
			break;
		default:
			break;
		}
	}

	return upgradedAttackStats;
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
					entity->CurrentDefenceStats.Armor = FMath::Clamp(entity->CurrentDefenceStats.Armor, 0, entity->CurrentDefenceStats.Armor);
					break;
				case EUpgradeStat::E_DAMAGE:
					entity->CurrentAttackStats.Damage += upgrade.Effect;
					entity->CurrentAttackStats.Damage = FMath::Clamp(entity->CurrentAttackStats.Damage, 0, entity->CurrentAttackStats.Damage);
					break;
				case EUpgradeStat::E_HEALTH:
					entity->CurrentDefenceStats.MaxHealth += upgrade.Effect;
					entity->Health += upgrade.Effect;
					entity->Health = FMath::Clamp(entity->Health, 1, entity->CurrentDefenceStats.MaxHealth);
					break;
				case EUpgradeStat::E_RANGE:
					entity->CurrentAttackStats.Range += upgrade.Effect;
					entity->CurrentAttackStats.Range = FMath::Clamp(entity->CurrentAttackStats.Range, 0.f, entity->CurrentAttackStats.Range);
					if (entity->CurrentAttackStats.Range >= entity->CurrentVisionStats.InnerRange)
					{
						entity->CurrentVisionStats.InnerRange = entity->CurrentAttackStats.Range;
						entity->CurrentVisionStats.OuterRange = entity->CurrentAttackStats.Range;
					}
					break;
				case EUpgradeStat::E_RATEOFFIRE:
					UE_LOG(RTS_TEAM_LOG, Warning, TEXT("CalculateUpgradeEffects > Flat upgrades don't work on rate of fire!"));
					break;
				case EUpgradeStat::E_SPEED:
					entity->CurrentMovementStats.Speed += upgrade.Effect;
					entity->CurrentMovementStats.Speed = FMath::Clamp(entity->CurrentMovementStats.Speed, 0.f, entity->CurrentMovementStats.Speed);
					break;
				case EUpgradeStat::E_VISION:
					entity->CurrentVisionStats.InnerRange += upgrade.Effect;
					entity->CurrentVisionStats.OuterRange += upgrade.Effect;
					entity->CurrentVisionStats.InnerRange = FMath::Clamp(entity->CurrentVisionStats.InnerRange, 0.f, entity->CurrentVisionStats.InnerRange);
					entity->CurrentVisionStats.OuterRange = FMath::Clamp(entity->CurrentVisionStats.OuterRange, 0.f, entity->CurrentVisionStats.OuterRange);
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
					}
				}
			}
			else
			{
				UE_LOG(RTS_TEAM_LOG, Warning, TEXT("CalculateUpgradeEffects > Non-percentual upgrade found!"));
			}
		}

		//Apply effect
		for (auto entity : Entities)
		{
			if (checkUpgrade.AffectedType != entity->EntityType)
				continue;

			switch (checkUpgrade.Stat)
			{
			case EUpgradeStat::E_ARMOR:
				entity->CurrentDefenceStats.Armor *= totalEffect;
				entity->CurrentDefenceStats.Armor = FMath::Clamp(entity->CurrentDefenceStats.Armor, 0, entity->CurrentDefenceStats.Armor);
				break;
			case EUpgradeStat::E_DAMAGE:
				entity->CurrentAttackStats.Damage *= totalEffect;
				entity->CurrentAttackStats.Damage = FMath::Clamp(entity->CurrentAttackStats.Damage, 0, entity->CurrentAttackStats.Damage);
				break;
			case EUpgradeStat::E_HEALTH:
				entity->CurrentDefenceStats.MaxHealth *= totalEffect;
				entity->Health *= totalEffect;
				entity->Health = FMath::Clamp(entity->Health, 1, entity->CurrentDefenceStats.MaxHealth);
				break;
			case EUpgradeStat::E_RANGE:
				entity->CurrentAttackStats.Range *= totalEffect;
				entity->CurrentAttackStats.Range = FMath::Clamp(entity->CurrentAttackStats.Range, 0.f, entity->CurrentAttackStats.Range);
				break;
			case EUpgradeStat::E_RATEOFFIRE:
				entity->CurrentAttackStats.AttackCooldown -= FMath::Clamp(entity->CurrentAttackStats.AttackCooldown * (1 - totalEffect), 0.0f, 
					entity->CurrentAttackStats.AttackCooldown);
				entity->CurrentAttackStats.AttackCooldown = FMath::Clamp(entity->CurrentAttackStats.AttackCooldown, 0.f, entity->CurrentAttackStats.AttackCooldown);
				entity-> AttackAdditionalAnimSpeed += totalEffect - 1;
				entity->AttackAdditionalAnimSpeed = FMath::Clamp(entity->AttackAdditionalAnimSpeed, 0.f, entity->AttackAdditionalAnimSpeed);
				break;
			case EUpgradeStat::E_SPEED:
				entity->CurrentMovementStats.Speed *= totalEffect;
				entity->CurrentMovementStats.Speed = FMath::Clamp(entity->CurrentMovementStats.Speed, 0.f, entity->CurrentMovementStats.Speed);
				break;
			case EUpgradeStat::E_VISION:
				entity->CurrentVisionStats.InnerRange *= totalEffect;
				entity->CurrentVisionStats.OuterRange *= totalEffect;
				entity->CurrentVisionStats.InnerRange = FMath::Clamp(entity->CurrentVisionStats.InnerRange, 0.f, entity->CurrentVisionStats.InnerRange);
				entity->CurrentVisionStats.OuterRange = FMath::Clamp(entity->CurrentVisionStats.OuterRange, 0.f, entity->CurrentVisionStats.OuterRange);
				break;
			default:
				break;
			}
		}
	}

}
