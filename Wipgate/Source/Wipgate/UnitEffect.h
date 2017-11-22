#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitEffect.generated.h"

class UParticleSystem;

UENUM(BlueprintType)
enum class EUnitEffectStat : uint8
{
	HEALING 	UMETA(DisplayName = "Healing"),
	DAMAGE 		UMETA(DisplayName = "Damage"),
};

UENUM(BlueprintType)
enum class EUnitEffectType : uint8
{
	LINEAR 		UMETA(DisplayName = "Apply gradually over time"),
	FADING 		UMETA(DisplayName = "Reduce over time"),
	INTENSIFY 	UMETA(DisplayName = "Increase over time"),
	AT_START 	UMETA(DisplayName = "Apply at start"),
	AT_END		UMETA(DisplayName = "Apply at end")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WIPGATE_API UUnitEffect : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUnitEffect();
	
	void Initialize(EUnitEffectStat stat, EUnitEffectType type, float intensity, float duration);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	float m_Intensity;
	float m_Duration;
	EUnitEffectStat m_AffectedStat;
	EUnitEffectType m_Type;

	UParticleSystem* m_StartParticles = nullptr;
	UParticleSystem* m_DuringParticles = nullptr;
	UParticleSystem* m_EndParticles = nullptr;
};
