
#include "Ability.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystem.h"

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

	UParticleSystem* ps = NewObject<UParticleSystem>(this, TEXT("PE_Hit"));
	FTransform trans;
	trans.SetLocation(FVector(0, 0, 10));
	UParticleSystemComponent* part =  UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ps, trans, true);
	UE_LOG(LogTemp, Warning, TEXT("Particle spawned"));
}

void AAbility::BeginPlay()
{
	Super::BeginPlay();
}

void AAbility::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

