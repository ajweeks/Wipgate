#include "UnitEffect.h"

void UUnitEffect::Initialize(const EUnitEffectStat stat, const EUnitEffectType type, const float delay, const int magnitude, const int duration)
{
	m_AffectedStat = stat;
	m_Type = type;
	m_Delay = delay;
	m_Elapsed = -delay; // start delay negative so effect can start at 0
	m_Magnitude = magnitude;
	m_Duration = duration;
}
