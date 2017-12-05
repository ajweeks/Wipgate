#pragma once

#include "CoreMinimal.h"
#include "Tile.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"

#include "Step.generated.h"

UCLASS(BlueprintType, Blueprintable)
class WIPGATE_API UStep: public UObject
{
	GENERATED_BODY()

public:
	UStep() {}
	virtual ~UStep() {}

	virtual void Execute(TMap<Tile*, UStaticMeshComponent*> blockOutMap) {};

protected:
	virtual void SetBlockColor(UStaticMeshComponent* block, FColor color);
	
	const int BLOCK_SIZE = 100;
};

/* -------------------------------------- */
UCLASS(BlueprintType, Blueprintable)
class WIPGATE_API UStep_Fill : public UStep
{
	GENERATED_BODY()

public:
	virtual ~UStep_Fill() {}
	void Initialize(Tile* tile, const bool isFilled)
	{
		m_Tile = tile; m_IsFilled = isFilled;
	}
	
	virtual void Execute(TMap<Tile*, UStaticMeshComponent*> blockOutMap)
	{
		UStaticMeshComponent* block = *blockOutMap.Find(m_Tile);
		if (m_IsFilled) {
			block->MoveComponent(FVector(0, 0, BLOCK_SIZE), FRotator(0), false);
			m_Tile->SetType(WALL);
			SetBlockColor(block, m_Tile->Color);
		}
		else {
			block->MoveComponent(FVector(0, 0, -BLOCK_SIZE), FRotator(0), false);
			m_Tile->SetType(FLOOR);
			SetBlockColor(block, m_Tile->Color);
		}
	}

private:
	Tile* m_Tile;
	bool m_IsFilled;
private:
	UStep_Fill() {};
};

/* -------------------------------------- */
UCLASS(BlueprintType, Blueprintable)
class WIPGATE_API UStep_Type : public UStep
{
	GENERATED_BODY()

public:
	virtual ~UStep_Type() {}
	void Initialize(Tile* tile, const TileType type)
	{
		m_Tile = tile; m_Type = type;
	}

	virtual void Execute(TMap<Tile*, UStaticMeshComponent*> blockOutMap)
	{
		m_Tile->SetType(m_Type);
		UStaticMeshComponent* block = *blockOutMap.Find(m_Tile);
		SetBlockColor(block, m_Tile->Color);	
	}

private:
	Tile* m_Tile;
	TileType m_Type;
private:
	UStep_Type() {};
};