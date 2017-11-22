#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Core//Public/Math/Vector2D.h"
#include "UnitEffect.h"
#include "Ability.generated.h"

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	E_TARGET_UNIT 	UMETA(DisplayName = "Target Unit"),
	E_TARGET_ENEMY 	UMETA(DisplayName = "Target Enemy"),
	E_TARGET_ALLY 	UMETA(DisplayName = "Target Ally"),
	E_TARGET_GROUND 	UMETA(DisplayName = "Target Ground"),
};

UENUM(BlueprintType)
enum class EAbilityState : uint8
{
	E_AVAILABLE 	UMETA(DisplayName = "Available"),
	E_SELECTED 	UMETA(DisplayName = "Selected"),
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
	UFUNCTION(BlueprintCallable, Category = "Ability Use Functions")
		void Select();
	UFUNCTION(BlueprintCallable, Category = "Ability Use Functions")
		virtual void Passive() {};

protected:
	/* protected non virtuals */
	UFUNCTION(BlueprintCallable, Category = "Ability Creation Functions")
		UUnitEffect* CreateUnitEffect(const EUnitEffectStat stat, const EUnitEffectType type, const int intensity, const int duration);

protected:
	/* general ability functionality */
	// get enemies/allies
	// get nearest
	// sort near to far
	// void PlaySound() {};
	// void SpawnParticle() {};
	// animation

	///* protected members */
	//AActor m_Caster;
	//AActor m_Target;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector2D m_TargetPos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability parameters")
		int m_Charges;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability parameters")
		float m_CooldownActive;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability parameters")
		float m_CooldownPassive;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability parameters")
		float m_CastTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability parameters")
		EAbilityType m_Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability parameters")
		EAbilityState m_State = EAbilityState::E_AVAILABLE;

	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	const float m_Interval = 0.5f;

};
