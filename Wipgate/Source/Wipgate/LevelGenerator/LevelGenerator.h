#pragma once

#include "CoreMinimal.h"
#include "LevelGrid.h"
#include "LevelBlockout.h"

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
		void ClearSteps() { m_Grid->ClearSteps(); }
	UFUNCTION(BlueprintCallable)
		TArray<UStep*> GetSteps() { return m_Grid->GetMainGrid()->GetSteps(); }
	
	UFUNCTION(BlueprintCallable)
		void Reset();

private:
	void ExecuteSteps();

	void GenerateBaseLayout(LevelGrid* grid, const int granularity);
	void FlagStreets();
	void CreateStreetsFromFlagged();
	void SparsifyStreetsRandom(const int odds);

private:
	LevelGrid* m_Grid = nullptr;
	ALevelBlockout* m_Blockout = nullptr;
	UStaticMesh* m_Block;
};
