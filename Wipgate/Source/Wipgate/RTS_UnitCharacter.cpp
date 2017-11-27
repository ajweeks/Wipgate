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

		// only apply effect after delay
		if (e->m_Elapsed < 0)
			continue;

		switch (e->m_Type)
		{
		case EUnitEffectType::OVER_TIME:
			ApplyEffectLinear(e);
			break;
		case EUnitEffectType::INSTANT:
			ApplyEffectOnce(e);
			break;
		default:
			break;
		}

		if (e->m_Ticks >= e->m_Duration)
			e->m_IsFinished = true;
	}

	/* Clean up effects */
	for (size_t i = m_UnitEffects.Num() - 1; i < m_UnitEffects.Num(); i--)
	{
		if (m_UnitEffects[i]->m_IsFinished)
			RemoveUnitEffect(m_UnitEffects[i]);
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

	// start particle systems
	if (effect->m_StartParticles)
		UGameplayStatics::SpawnEmitterAttached(effect->m_StartParticles, RootComponent);

	if (effect->m_ConstantParticles)
		effect->StartParticleConstant(RootComponent);
}

void ARTS_UnitCharacter::RemoveUnitEffect(UUnitEffect * effect)
{
	if (!m_UnitEffects.Contains(effect))
		return;

	switch (effect->m_AffectedStat)
	{
	case EUnitEffectStat::ARMOR:
		if(effect->m_Type == EUnitEffectType::OVER_TIME)
			m_UnitCoreComponent->CurrentArmor -= (effect->m_Magnitude / effect->m_Duration) * effect->m_Ticks;
		else
			m_UnitCoreComponent->CurrentArmor -= effect->m_Magnitude;
		break;
	case EUnitEffectStat::MOVEMENT_SPEED:
		break;
	default:
		break;
	}
	m_UnitEffects.Remove(effect);

	// stop particle systems
	if (effect->m_EndParticles)
		UGameplayStatics::SpawnEmitterAttached(effect->m_EndParticles, RootComponent);

	if (effect->m_ConstantParticles)
		effect->StopParticleConstant();
}

void ARTS_UnitCharacter::ApplyEffectLinear(UUnitEffect * effect)
{
	if (effect->m_Elapsed > EFFECT_INTERVAL)
	{
		// increment ticks each interval
		effect->m_Ticks++;
		if (effect->m_IsFinished)
			return;

		// only apply effect if it was not finished yet
		switch (effect->m_AffectedStat)
		{
		case EUnitEffectStat::ARMOR:
			m_UnitCoreComponent->CurrentArmor += effect->m_Magnitude / (effect->m_Duration / EFFECT_INTERVAL);
			break;
		case EUnitEffectStat::DAMAGE:
			m_UnitCoreComponent->ApplyDamage_CPP(effect->m_Magnitude / (effect->m_Duration / EFFECT_INTERVAL), false);
			break;
		case EUnitEffectStat::HEALING:
			m_UnitCoreComponent->ApplyHealing(effect->m_Magnitude / (effect->m_Duration / EFFECT_INTERVAL));
			break;
		case EUnitEffectStat::MOVEMENT_SPEED:
			break;
		default:
			break;
		}

		// spawn particles and reset time
		if (effect->m_TickParticles)
			UGameplayStatics::SpawnEmitterAttached(effect->m_TickParticles, RootComponent);
		effect->m_Elapsed = 0;
	}
}

void ARTS_UnitCharacter::ApplyEffectOnce(UUnitEffect * effect)
{
	if (effect->m_Ticks == 0)
	{
		effect->m_Ticks++;
		if (effect->m_TickParticles)
			UGameplayStatics::SpawnEmitterAttached(effect->m_TickParticles, RootComponent);

		switch (effect->m_AffectedStat)
		{
		case EUnitEffectStat::ARMOR:
			m_UnitCoreComponent->CurrentArmor += effect->m_Magnitude;
			break;

		case EUnitEffectStat::DAMAGE:
			m_UnitCoreComponent->ApplyDamage_CPP(effect->m_Magnitude, false);
			break;

		case EUnitEffectStat::HEALING:
			m_UnitCoreComponent->ApplyHealing(effect->m_Magnitude);
			break;

		case EUnitEffectStat::MOVEMENT_SPEED:
			break;

		default:
			break;
		}
	}

	if (effect->m_Elapsed > effect->m_Duration)
		effect->m_IsFinished = true;
}
