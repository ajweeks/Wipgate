
#include "Ability.h"
#include "UnitEffect.h"

AAbility::AAbility()
{
	PrimaryActorTick.bCanEverTick = true;
}

UUnitEffect* AAbility::CreateUnitEffect(const EUnitEffectStat stat, const EUnitEffectType type, 
	const float delay, const int magnitude, const int duration)
{
	UUnitEffect* unitEffect = NewObject<UUnitEffect>(this);
	unitEffect->Initialize(stat, type, delay, magnitude, duration);
	return unitEffect;
}

void AAbility::BeginPlay()
{
	Super::BeginPlay();
}

void AAbility::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Passive();
}

