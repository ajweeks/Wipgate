// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTS_Entity.h"
#include "RTS_Structure.generated.h"

class UStaticMeshComponent;

UCLASS()
class WIPGATE_API ARTS_Structure : public ARTS_Entity
{
	GENERATED_BODY()
public:

	ARTS_Structure();

	UFUNCTION(BlueprintCallable, Category = "Team")
	virtual void SetTeamMaterial() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	UStaticMeshComponent* StaticMeshComponent;

private:
	
	
};
