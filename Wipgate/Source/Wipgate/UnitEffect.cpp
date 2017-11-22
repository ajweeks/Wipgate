#include "UnitEffect.h"

UUnitEffect::UUnitEffect()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUnitEffect::Initialize(EUnitEffectStat stat, EUnitEffectType type, float intensity, float duration)
{
	m_AffectedStat = stat;
	m_Type = type;
	m_Intensity = intensity;
	m_Duration = duration;
}

void UUnitEffect::BeginPlay()
{
	Super::BeginPlay();
}

void UUnitEffect::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

