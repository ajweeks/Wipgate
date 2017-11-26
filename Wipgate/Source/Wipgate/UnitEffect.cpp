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

//UUnitEffect * UUnitEffect::GetCopy()
//{
//	UUnitEffect* copy = NewObject<UUnitEffect>(this);
//	copy->m_AffectedStat = this->m_AffectedStat;
//	copy->m_Type = this->m_Type;
//	copy->m_Elapsed = this->m_Elapsed;
//	copy->m_Delay = this->m_Delay;
//	copy->m_Duration = this->m_Duration;
//	copy->m_IsFinished = this->m_IsFinished;
//	copy->m_Magnitude = this->m_Magnitude;
//	copy->m_TickParticles = this->m_TickParticles;
//	copy->m_StartParticles = this->m_StartParticles;
//	copy->m_EndParticles = this->m_EndParticles;
//	return nullptr;
//}
