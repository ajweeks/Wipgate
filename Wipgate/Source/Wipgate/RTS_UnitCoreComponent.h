// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RTS_UnitCoreComponent.generated.h"


UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WIPGATE_API URTS_UnitCoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URTS_UnitCoreComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//ISDEAD
	UPROPERTY(BlueprintReadWrite)
	bool IsDead = false;

	//ATTACK PROPERTIES
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attack")
	int BaseDamage = 10.f;
	UPROPERTY(BlueprintReadWrite)
	int CurrentDamage = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float BaseRateOfFire = 10.f;
	UPROPERTY(BlueprintReadWrite)
	float CurrentRateOfFire = 0.f;
	UPROPERTY(BlueprintReadWrite)
	float TimerRateOfFire = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float BaseAttackRange = 250;
	UPROPERTY(BlueprintReadWrite)
	float CurrentAttackRange = 250;

	
};
