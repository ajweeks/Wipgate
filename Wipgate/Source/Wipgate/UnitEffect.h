#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedStruct.h"
#include "UnitEffect.generated.h"

class UParticleSystem;
class UParticleSystemComponent;

//TODO: E_ naming convention
UENUM(BlueprintType)
enum class EUnitEffectStat : uint8
{
	HEALING 		UMETA(DisplayName = "Healing"),
	DAMAGE 			UMETA(DisplayName = "Damage"),
	ARMOR			UMETA(DisplayName = "Armor"),
	MOVEMENT_SPEED	UMETA(DisplayName = "Movement speed"),
	ATTACK_RATE		UMETA(DisplayName = "Attack rate"),
	ATTACK_DAMAGE	UMETA(DisplayName = "Attack damage"),
};

//TODO: E_ naming convention
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
		void SetParticles(UParticleSystem* tick, UParticleSystem* start, UParticleSystem* end, UParticleSystem* constant, FName socketName);
	UFUNCTION(BlueprintCallable)
		void StartParticleConstant(USceneComponent* comp);
	void StopParticleConstant();
	void AttachParticleToSocket(USceneComponent * skeletalMesh);

	//UFUNCTION(BlueprintCallable)
	//UUnitEffect* GetCopy();

public:
	float Elapsed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Status")
	int Ticks = 0;
	bool IsFinished = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
		int Magnitude;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
		float Delay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
		int Duration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
		EUnitEffectStat AffectedStat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
		EUnitEffectType Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
		FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect parameters")
		FName Tag;

	UParticleSystem* StartParticles = nullptr;
	UParticleSystem* TickParticles = nullptr;
	UParticleSystem* EndParticles = nullptr;
	UParticleSystem* ConstantParticles = nullptr;
	UParticleSystemComponent * ParticleComponent = nullptr;
};
