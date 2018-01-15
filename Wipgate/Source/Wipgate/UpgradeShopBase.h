// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UpgradeShopBase.generated.h"

class ARTS_PlayerController;
class UBoxComponent;

UCLASS()
class WIPGATE_API AUpgradeShopBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AUpgradeShopBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* OverlapCube;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UBoxComponent* BoxCollider;

private:
	ARTS_PlayerController* m_PlayerControllerRef;
};
