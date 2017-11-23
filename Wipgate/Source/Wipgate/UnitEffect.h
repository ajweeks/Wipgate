#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedStruct.h"
#include "UnitEffect.generated.h"

class UParticleSystem;

UENUM(BlueprintType)
enum class EUnitEffectStat : uint8
{
	HEALING 		UMETA(DisplayName = "Healing"),
	DAMAGE 			UMETA(DisplayName = "Damage"),
	ARMOR			UMETA(DisplayName = "Armor"),
	MOVEMENT_SPEED	UMETA(DisplayName = "Movement speed"),
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
class WIPGATE_API UUnitEffect : public UUserDefinedStruct
{
	GENERATED_BODY()

public:	
	UUnitEffect() {};
	
	void Initialize(EUnitEffectStat stat, EUnitEffectType type, float intensity, float duration);
	UFUNCTION(BlueprintCallable)
	void SetTickParticle(UParticleSystem* particle) { m_TickParticles = particle; }
	
public:
	float m_Elapsed = 0;
	int m_Ticks = 0;
	bool m_IsFinished = false;

	int m_Intensity;
	int m_Duration;
	EUnitEffectStat m_AffectedStat;
	EUnitEffectType m_Type;

	UParticleSystem* m_StartParticles = nullptr;
	UParticleSystem* m_TickParticles = nullptr;
	UParticleSystem* m_EndParticles = nullptr;
};
