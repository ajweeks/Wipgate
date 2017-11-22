
#include "Ability.h"
#include "UnitEffect.h"

AAbility::AAbility()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAbility::Select()
{
	if (m_State == EAbilityState::E_SELECTED)
	{
		m_State = EAbilityState::E_AVAILABLE;
		UE_LOG(LogTemp, Warning, TEXT("Available"));
	}
	else if (m_State == EAbilityState::E_AVAILABLE)
	{
		m_State = EAbilityState::E_SELECTED;
		UE_LOG(LogTemp, Warning, TEXT("Selected"))
	}
	// display cursor/decals
}

UUnitEffect * AAbility::CreateUnitEffect(EUnitEffectStat stat, EUnitEffectType type, float intensity, float duration)
{
	UUnitEffect* unitEffect = NewObject<UUnitEffect>(this);
	unitEffect->Initialize(stat, type, intensity, duration);
	return unitEffect;
}

void AAbility::BeginPlay()
{
	Super::BeginPlay();
}

void AAbility::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

