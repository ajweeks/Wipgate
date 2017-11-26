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
	OVER_TIME 		UMETA(DisplayName = "Over time"),
	//FADING 		UMETA(DisplayName = "Reduce over time"),
	//INTENSIFY 	UMETA(DisplayName = "Increase over time"),
	INSTANT 	UMETA(DisplayName = "Apply at start"),
	//AT_END		UMETA(DisplayName = "Apply at end")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WIPGATE_API UUnitEffect : public UUserDefinedStruct
{
	GENERATED_BODY()

public:	
	UUnitEffect() {};
	
	void Initialize(const EUnitEffectStat stat, const EUnitEffectType type, const float delay, const int magnitude, const int duration);
	UFUNCTION(BlueprintCallable)
	void SetTickParticle(UParticleSystem* particle) { m_TickParticles = particle; }
	//UFUNCTION(BlueprintCallable)
	//UUnitEffect* GetCopy();

public:
	float m_Elapsed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Status")
	int m_Ticks = 0;
	bool m_IsFinished = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
	int m_Magnitude;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
	float m_Delay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
	int m_Duration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
	EUnitEffectStat m_AffectedStat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
	EUnitEffectType m_Type;

	UParticleSystem* m_StartParticles = nullptr;
	UParticleSystem* m_TickParticles = nullptr;
	UParticleSystem* m_EndParticles = nullptr;
};
