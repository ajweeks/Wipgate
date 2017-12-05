#pragma once

#include "CoreMinimal.h"
#include "LevelGrid.h"

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

	/* --- Initialization --- */
	UFUNCTION(BlueprintCallable)
		void SetBasicBlock(UStaticMesh* mesh) { m_Block = mesh; }
	UFUNCTION(BlueprintCallable)
		void InitializeBlockout();

	/* --- Generation --- */
	UFUNCTION(BlueprintCallable)
		void GenerateStreets(const int granularity);

	/* --- Steps --- */
	UFUNCTION(BlueprintCallable)
		void ExecuteStep(UStep* step);
	UFUNCTION(BlueprintCallable)
		TArray<UStep*> GetSteps() { return m_Grid->GetMainGrid()->GetSteps(); }
	
	void UpdateBlock(Tile* tile);

private:
	LevelGrid* m_Grid = nullptr;
	UStaticMesh* m_Block;
	TMap<Tile*, UStaticMeshComponent*> m_BlockOutMap;

	const int BLOCK_SIZE = 100;
};
