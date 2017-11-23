// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_UnitCharacter.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystem.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"

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
	for (auto e : m_UnitEffects)
	{
		e->m_Elapsed += DeltaTime;
		// TODO: no intervals for more gradual effects like snares

		// tick effects every second
		if (e->m_Type == EUnitEffectType::LINEAR)
		{
			if (e->m_Elapsed > EFFECT_INTERVAL)
			{
				// increment ticks each interval
				e->m_Ticks++;
				if (e->m_IsFinished)
					continue;

				// only apply effect if it was not finished yet
				ApplyEffect(e);

				e->m_Elapsed = 0;
			}
		}

		// effects without tick
		if (e->m_Type == EUnitEffectType::AT_START)
		{
			if (e->m_AffectedStat == EUnitEffectStat::ARMOR)
			{
				m_UnitCoreComponent->CurrentArmor += e->m_Intensity;
				e->m_IsFinished = true;
			}
		}

		if (e->m_Ticks >= e->m_Duration)
			e->m_IsFinished = true;
	}

	/* Clean up effects */
	for (size_t i = m_UnitEffects.Num() - 1; i < m_UnitEffects.Num(); i--)
	{
		if (m_UnitEffects[i]->m_IsFinished)
			m_UnitEffects.RemoveAt(i);
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

void ARTS_UnitCharacter::ApplyEffect(UUnitEffect * effect)
{
	if (effect->m_AffectedStat == EUnitEffectStat::DAMAGE)
	{
		if (effect->m_Type == EUnitEffectType::LINEAR)
		{
			m_UnitCoreComponent->ApplyDamage_CPP(effect->m_Intensity / (effect->m_Duration / EFFECT_INTERVAL), false);
			if (effect->m_TickParticles)
				UGameplayStatics::SpawnEmitterAttached(effect->m_TickParticles, RootComponent);
		}
	}

	if (effect->m_AffectedStat == EUnitEffectStat::HEALING)
	{
		if (effect->m_Type == EUnitEffectType::LINEAR)
		{
			m_UnitCoreComponent->CurrentHealth += effect->m_Intensity / (effect->m_Duration / EFFECT_INTERVAL);
			if (effect->m_TickParticles)
				UGameplayStatics::SpawnEmitterAttached(effect->m_TickParticles, RootComponent);
		}
	}

	if (effect->m_AffectedStat == EUnitEffectStat::ARMOR)
	{
		if (effect->m_Type == EUnitEffectType::AT_START)
		{
			m_UnitCoreComponent->CurrentArmor += effect->m_Intensity / (effect->m_Duration / EFFECT_INTERVAL);
			if(effect->m_TickParticles)
				UGameplayStatics::SpawnEmitterAttached(effect->m_TickParticles, RootComponent);
		}
	}
}
