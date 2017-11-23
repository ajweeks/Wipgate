// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_UnitCoreComponent.h"


// Sets default values for this component's properties
URTS_UnitCoreComponent::URTS_UnitCoreComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URTS_UnitCoreComponent::BeginPlay()
{
	Super::BeginPlay();

	//Attack
	CurrentDamage = BaseDamage;
	CurrentRateOfFire = BaseRateOfFire;
	CurrentAttackRange = BaseAttackRange;
	TimerRateOfFire = 0.f;

	//Defense
	CurrentArmor = BaseArmor;
	CurrentHealth = BaseHealth;

	//Vision
	CurrentOuterCircleRange = BaseOuterCircleRange;
	CurrentInnerCircleRange = BaseInnerCircleRange;
}


// Called every frame
void URTS_UnitCoreComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
