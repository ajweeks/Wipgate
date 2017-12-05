
#include "Ability.h"
#include "UnitEffect.h"
#include "Engine/Engine.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text)

AAbility::AAbility()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAbility::SetTarget(AActor* Target)
{
	m_Target = Target;
}

UUnitEffect* AAbility::CreateUnitEffect(const EUnitEffectStat stat, const EUnitEffectType effectType, const float delay, const int magnitude, const int duration)
{
	UUnitEffect* unitEffect = NewObject<UUnitEffect>(this);
	unitEffect->Initialize(stat, effectType, delay, magnitude, duration);
	return unitEffect;
}

void AAbility::BeginPlay()
{
	Super::BeginPlay();
}

void AAbility::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CooldownPassiveElapsed >= CooldownPassive)
		Passive();
}

