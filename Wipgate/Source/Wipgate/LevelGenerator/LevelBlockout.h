#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGrid.h"
#include "Tile.h"
#include "LevelBlockout.generated.h"

UCLASS()
class WIPGATE_API ALevelBlockout : public AActor
{
	GENERATED_BODY()
	
public:	
	ALevelBlockout();
	~ALevelBlockout() {}

	void Initialize(LevelGrid* grid);
	void SetBlock(UStaticMesh* block) { m_Block = block; }
	TMap<Tile*, UStaticMeshComponent*> GetBlockoutMap() { return m_BlockOutMap; }

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	TMap<Tile*, UStaticMeshComponent*> m_BlockOutMap;
	UStaticMesh* m_Block;
	const int BLOCK_SIZE = 100;
};
