#pragma once

#include "CoreMinimal.h"
#include "BaseGrid.h"

#include "LevelGenerator.generated.h"

UCLASS()
class WIPGATE_API ALevelGenerator: public AActor
{
	GENERATED_BODY()

public:
	ALevelGenerator() {}
	~ALevelGenerator() {}
protected:
	virtual void BeginPlay() override {};

public:
	virtual void Tick(float DeltaTime) override {};

	UFUNCTION(BlueprintCallable)
		void SetBasicBlock(UStaticMesh* mesh) { m_Block = mesh; }
	UFUNCTION(BlueprintCallable)
		void InitializeBlockout();
	UFUNCTION(BlueprintCallable)
		void EmptyRandomTile();
	
	void UpdateBlock(Tile* tile);

private:
	BaseGrid* m_Grid = nullptr;
	UStaticMesh* m_Block;
	TMap<Tile*, UStaticMeshComponent*> m_BlockOutMap;

	const int BLOCK_SIZE = 100;
};
