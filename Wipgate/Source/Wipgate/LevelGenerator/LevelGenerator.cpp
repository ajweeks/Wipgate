#include "LevelGenerator.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Wipgate.h"
#include "Step.h"

void ALevelGenerator::InitializeBlockout()
{
	m_Grid = new LevelGrid(25, 25, this);
	m_Grid->SetMainGrid(m_Grid);
	FVector origin(-m_Grid->GetWidth() / 2 * BLOCK_SIZE, -m_Grid->GetHeight() / 2 * BLOCK_SIZE, 0);

	for (size_t col = 0; col < m_Grid->GetWidth(); col++)
	{
		for (size_t row = 0; row < m_Grid->GetHeight(); row++)
		{
			FVector pos(col * BLOCK_SIZE, row * BLOCK_SIZE, 0);
			UStaticMeshComponent * meshComp = NewObject<UStaticMeshComponent>(this);
			meshComp->SetStaticMesh(m_Block);
			meshComp->SetWorldLocation(origin + pos);

			m_BlockOutMap.Add((*m_Grid)[col][row], meshComp);

			FAttachmentTransformRules rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
			meshComp->AttachToComponent(RootComponent, rules);
			meshComp->RegisterComponentWithWorld(GetWorld());
		}
	}
}

void ALevelGenerator::GenerateStreets(const int granularity)
{
	// empty grid
	vector<Tile*> tiles = m_Grid->GetTiles();
	m_Grid->SetFilledTiles(tiles, false);
	for (auto step : m_Grid->GetSteps())
		ExecuteStep(step);
	m_Grid->ClearSteps();

	// make building blocks
	m_Grid->SplitDeep(granularity);
	auto children = m_Grid->GetChildrenDeep();
	for (auto c : children)
	{
		c->SetFilledArea(1, 1, c->GetHeight() - 2, c->GetWidth() - 2, true);
	}

	// flag streets
	vector<Tile*> emptyTiles = m_Grid->GetTilesFilled(tiles, false);
	for (auto t : emptyTiles)
	{
		vector<Tile*> nearby = m_Grid->GetNearbyTiles(t->GetPosition());
		vector<Tile*> nearbyFilleds = m_Grid->GetTilesFilled(nearby, true);
		if (nearbyFilleds.size() > 1)
			m_Grid->SetType(t, STREET);
	}

	vector<Tile*> intersectionTiles = m_Grid->GetTilesWithType(tiles, FLOOR);
	Tile* blockCorner;
	vector<Tile*> oppositeTiles;
	for (auto t : intersectionTiles)
	{
		// get nearby building block corner
		blockCorner = m_Grid->GetTilesFilled(m_Grid->GetNearbyTiles(t), true).front();
		Direction cornerDir = m_Grid->GetTileDirection(t, blockCorner);
		oppositeTiles = m_Grid->GetOppositeDirTiles(t, cornerDir);
		m_Grid->SetTilesType(oppositeTiles, FLOOR);
	}
}

void ALevelGenerator::ExecuteStep(UStep * step)
{
	if(step)
		step->Execute(m_BlockOutMap);
}

void ALevelGenerator::UpdateBlock(Tile * tile)
{
	FVector origin(-m_Grid->GetWidth() / 2 * BLOCK_SIZE, -m_Grid->GetHeight() / 2 * BLOCK_SIZE, 0);

	UStaticMeshComponent* block = *m_BlockOutMap.Find(tile);
	if (tile->IsFilled)
		block->SetWorldLocation(FVector(origin.X + tile->X * BLOCK_SIZE, origin.Y + tile->Y * BLOCK_SIZE, 0));
	else
		block->SetWorldLocation(FVector(origin.X + tile->X * BLOCK_SIZE, origin.Y + tile->Y * BLOCK_SIZE, -BLOCK_SIZE));
}