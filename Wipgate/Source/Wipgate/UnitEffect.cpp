#include "UnitEffect.h"

void UUnitEffect::Initialize(EUnitEffectStat stat, EUnitEffectType type, float intensity, float duration)
{
	m_AffectedStat = stat;
	m_Type = type;
	m_Intensity = intensity;
	m_Duration = duration;
}
