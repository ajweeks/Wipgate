// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_UnitCharacter.h"

DEFINE_LOG_CATEGORY(RTS_Unit_Log);

ARTS_UnitCharacter::ARTS_UnitCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ARTS_UnitCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ARTS_UnitCharacter::SetSelected_Implementation(bool selected)
{
	Selected = selected;
}

void ARTS_UnitCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/* Apply effects */
	m_ElapsedInterval += DeltaTime;
	if (m_ElapsedInterval > EFFECT_INTERVAL)
	{
		for (auto e : m_UnitEffects)
		{
			if (e->m_IsFinished)
				continue;

			e->m_Elapsed += m_ElapsedInterval;

			if (e->m_AffectedStat == EUnitEffectStat::DAMAGE)
			{
				if (e->m_Type == EUnitEffectType::LINEAR)
				{
					m_UnitCoreComponent->ApplyDamage_CPP(e->m_Intensity / (e->m_Duration / EFFECT_INTERVAL), false);
				}
			}


			if (e->m_Elapsed > e->m_Duration)
				e->m_IsFinished = true;
		}

		m_ElapsedInterval = 0;
	}
}

bool ARTS_UnitCharacter::IsSelected() const
{
	return Selected;
}

TArray<UUnitEffect*> ARTS_UnitCharacter::GetUnitEffects() const
{
	return m_UnitEffects;
}

void ARTS_UnitCharacter::AddUnitEffect(UUnitEffect * effect)
{
	m_UnitEffects.Add(effect);
}
