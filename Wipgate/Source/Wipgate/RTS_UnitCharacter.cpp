// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_UnitCharacter.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(RTS_Unit_Log);

ARTS_UnitCharacter::ARTS_UnitCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilityIcons.SetNumZeroed(NUM_ABILITIES);
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
	for (auto e : UnitEffects)
	{
		e->Elapsed += DeltaTime;

		// only apply effect after delay
		if (e->Elapsed < 0)
			continue;

		switch (e->Type)
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

		if (e->Ticks >= e->Duration)
			e->IsFinished = true;
	}

	/* Clean up effects */
	for (size_t i = UnitEffects.Num() - 1; i < UnitEffects.Num(); i--)
	{
		if (UnitEffects[i]->IsFinished)
			RemoveUnitEffect(UnitEffects[i]);
	}
}

bool ARTS_UnitCharacter::IsSelected() const
{
	return Selected;
}

TArray<UUnitEffect*> ARTS_UnitCharacter::GetUnitEffects() const
{
	return UnitEffects;
}

void ARTS_UnitCharacter::AddUnitEffect(UUnitEffect * effect)
{
	UnitEffects.Add(effect);

	// start particle systems
	if (effect->StartParticles)
		UGameplayStatics::SpawnEmitterAttached(effect->StartParticles, RootComponent);

	if (effect->ConstantParticles)
		effect->StartParticleConstant(RootComponent);
}

void ARTS_UnitCharacter::RemoveUnitEffect(UUnitEffect * effect)
{
	if (!UnitEffects.Contains(effect))
		return;

	switch (effect->AffectedStat)
	{
	case EUnitEffectStat::ARMOR:
		if(effect->Type == EUnitEffectType::OVER_TIME)
			UnitCoreComponent->CurrentArmor -= (effect->Magnitude / effect->Duration) * effect->Ticks;
		else
			UnitCoreComponent->CurrentArmor -= effect->Magnitude;
		break;
	case EUnitEffectStat::MOVEMENT_SPEED:
		break;
	default:
		break;
	}
	UnitEffects.Remove(effect);

	// stop particle systems
	if (effect->EndParticles)
		UGameplayStatics::SpawnEmitterAttached(effect->EndParticles, RootComponent);

	if (effect->ConstantParticles)
		effect->StopParticleConstant();
}

void ARTS_UnitCharacter::DisableDebug()
{
	for (UStaticMeshComponent* debugMesh : DebugMeshes)
	{
		UActorComponent* actor = Cast<UActorComponent>(debugMesh);
		actor->DestroyComponent();
	}
	DebugMeshes.Empty();
}

void ARTS_UnitCharacter::ApplyEffectLinear(UUnitEffect * effect)
{
	if (effect->Elapsed > EFFECT_INTERVAL)
	{
		// increment ticks each interval
		effect->Ticks++;
		if (effect->IsFinished)
			return;

		// only apply effect if it was not finished yet
		switch (effect->AffectedStat)
		{
		case EUnitEffectStat::ARMOR:
			UnitCoreComponent->CurrentArmor += effect->Magnitude / (effect->Duration / EFFECT_INTERVAL);
			break;
		case EUnitEffectStat::DAMAGE:
			UnitCoreComponent->ApplyDamage_CPP(effect->Magnitude / (effect->Duration / EFFECT_INTERVAL), false);
			break;
		case EUnitEffectStat::HEALING:
			UnitCoreComponent->ApplyHealing(effect->Magnitude / (effect->Duration / EFFECT_INTERVAL));
			break;
		case EUnitEffectStat::MOVEMENT_SPEED:
			break;
		default:
			break;
		}

		// spawn particles and reset time
		if (effect->TickParticles)
			UGameplayStatics::SpawnEmitterAttached(effect->TickParticles, RootComponent);
		effect->Elapsed = 0;
	}
}

void ARTS_UnitCharacter::ApplyEffectOnce(UUnitEffect * effect)
{
	if (effect->Ticks == 0)
	{
		effect->Ticks++;
		if (effect->TickParticles)
			UGameplayStatics::SpawnEmitterAttached(effect->TickParticles, RootComponent);

		switch (effect->AffectedStat)
		{
		case EUnitEffectStat::ARMOR:
			UnitCoreComponent->CurrentArmor += effect->Magnitude;
			break;

		case EUnitEffectStat::DAMAGE:
			UnitCoreComponent->ApplyDamage_CPP(effect->Magnitude, false);
			break;

		case EUnitEffectStat::HEALING:
			UnitCoreComponent->ApplyHealing(effect->Magnitude);
			break;

		case EUnitEffectStat::MOVEMENT_SPEED:
			break;

		default:
			break;
		}
	}

	if (effect->Elapsed > effect->Duration)
		effect->IsFinished = true;
}
