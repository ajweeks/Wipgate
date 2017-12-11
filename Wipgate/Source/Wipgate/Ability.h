#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/Vector2D.h"
#include "UnitEffect.h"
#include "Ability.generated.h"

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	E_TARGET_UNIT 		UMETA(DisplayName = "Target Unit"),
	E_TARGET_ENEMY 		UMETA(DisplayName = "Target Enemy"),
	E_TARGET_ALLY 		UMETA(DisplayName = "Target Ally"),
	E_TARGET_GROUND 	UMETA(DisplayName = "Target Ground"),
	E_SELF				UMETA(DisplayName = "Self")
};

UENUM(BlueprintType)
enum class EAbilityState : uint8
{
	E_AVAILABLE 	UMETA(DisplayName = "Available"),
	E_SELECTED 		UMETA(DisplayName = "Selected"),
	E_ON_COOLDOWN 	UMETA(DisplayName = "On cooldown"),
};

UCLASS()
class WIPGATE_API AAbility : public AActor
{
	GENERATED_BODY()

public:
	AAbility();

	/* sandbox methods */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Ability Use Functions")
		void Activate();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Ability Use Functions")
		void Passive();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Ability Use Functions")
		void Select();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Ability Use Functions")
		void Deselect();

	void SetTarget(AActor* Target);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability parameters")
		EAbilityType Type = EAbilityType::E_TARGET_UNIT;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability parameters")
		EAbilityState State = EAbilityState::E_AVAILABLE;

	/* --- Ability parameters --- */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability parameters")
		float CastRange;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability parameters")
		int Charges;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability parameters")
		float Cooldown;
	UPROPERTY(BlueprintReadWrite, Category = "Ability state")
		float CooldownElapsed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability parameters")
		float CooldownPassive;
	UPROPERTY(BlueprintReadWrite, Category = "Ability state")
		float CooldownPassiveElapsed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability parameters")
		float CastTime;

	// True once this ability has finished executing
	UPROPERTY(BlueprintReadWrite)
		bool IsCompleted;

protected:
	/* protected non virtuals */
	UFUNCTION(BlueprintCallable, Category = "Ability Creation Functions")
		UUnitEffect* CreateUnitEffect(const EUnitEffectStat stat, const EUnitEffectType effectType, const float delay, const int magnitude, const int duration);

protected:
	/* protected members */
	UPROPERTY(BlueprintReadWrite, Category = "Ability parameters")
		AActor* m_Caster;
	UPROPERTY(BlueprintReadWrite, Category = "Ability parameters")
		AActor* m_Target;

	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
