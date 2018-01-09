// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTS_Entity.h"
#include "RTS_Unit.generated.h"

class USkeletalMeshComponent;
class AAbility;

UCLASS()
class WIPGATE_API ARTS_Unit : public ARTS_Entity
{
	GENERATED_BODY()
	
public:
	ARTS_Unit();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Team")
	virtual void SetTeamMaterial() override;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void Kill() override;

	// True when this unit is inside the level goal collider (through the gate)
	UPROPERTY(BlueprintReadWrite)
		bool InLevelGoal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
		TSubclassOf<AAbility> AbilityAttackClass;

	//Minimum amount of currency this unit can drop
	UPROPERTY(EditAnywhere)
		int MinimumCurrencyDrop = 0;

	//Maximum amount of currency this unit can drop
	UPROPERTY(EditAnywhere)
		int MaximumCurrencyDrop = 0;

	//Class to spawn when unit dies
	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> CurrencyEffectClass;
private:
	
	float m_PostKillTimer = 0;
	const float POSTKILLDELAY = 3;
};
