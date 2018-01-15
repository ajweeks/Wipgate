// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UpgradeShopBase.generated.h"

class ARTS_PlayerController;

UCLASS()
class WIPGATE_API AUpgradeShopBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AUpgradeShopBase();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* OverlapCube;

private:
	void OnOverlapCubeBeginCursorOver(UPrimitiveComponent* Component);
	void OnOverlapCubeEndCursorOver(UPrimitiveComponent* Component);
	
	ARTS_PlayerController* m_PlayerControllerRef;
};
