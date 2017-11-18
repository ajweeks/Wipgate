// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_UnitCharacter.h"

DEFINE_LOG_CATEGORY(RTS_Unit_Log);

ARTS_UnitCharacter::ARTS_UnitCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARTS_UnitCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARTS_UnitCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARTS_UnitCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ARTS_UnitCharacter::SetSelected_Implementation(bool selected)
{
	Selected = selected;
	UE_LOG(RTS_Unit_Log, Log, TEXT("Unit selected: %b"), selected);

}

bool ARTS_UnitCharacter::IsSelected() const
{
	return Selected;
}

