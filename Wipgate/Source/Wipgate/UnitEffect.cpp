#include "UnitEffect.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"

void UUnitEffect::Initialize(const EUnitEffectStat stat, const EUnitEffectType type, const float delay, const int magnitude, const int duration)
{
	AffectedStat = stat;
	Type = type;
	Delay = delay;
	Elapsed = -delay; // start delay negative so effect can start at 0
	Magnitude = magnitude;
	Duration = duration;
}

void UUnitEffect::SetParticles(UParticleSystem * tick, UParticleSystem * start, UParticleSystem * end, UParticleSystem* constant)
{
	TickParticles = tick;
	StartParticles = start;
	EndParticles = end;
	ConstantParticles = constant;
}

void UUnitEffect::StartParticleConstant(USceneComponent* comp)
{
	if(!ParticleComponent)
		ParticleComponent = UGameplayStatics::SpawnEmitterAttached(ConstantParticles, comp);
}

void UUnitEffect::StopParticleConstant()
{
	if(ParticleComponent)
		ParticleComponent->Deactivate();
}

//UUnitEffect * UUnitEffect::GetCopy()
//{
//	UUnitEffect* copy = NewObject<UUnitEffect>(this);
//	copy->AffectedStat = this->AffectedStat;
//	copy->Type = this->Type;
//	copy->Elapsed = this->Elapsed;
//	copy->Delay = this->Delay;
//	copy->Duration = this->Duration;
//	copy->IsFinished = this->IsFinished;
//	copy->Magnitude = this->Magnitude;
//	copy->TickParticles = this->TickParticles;
//	copy->StartParticles = this->StartParticles;
//	copy->EndParticles = this->EndParticles;
//	return nullptr;
//}
