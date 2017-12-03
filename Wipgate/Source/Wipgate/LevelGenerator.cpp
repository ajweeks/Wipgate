#include "LevelGenerator.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "Runtime/Engine/Classes/Engine/World.h"

void ALevelGenerator::InitializeBlockout()
{
	m_Grid = new BaseGrid(5, 5);
	FVector origin(-m_Grid->GetWidth() / 2 * BLOCK_SIZE, -m_Grid->GetHeight() / 2 * BLOCK_SIZE, 0);

	for (size_t col = 0; col < m_Grid->GetWidth(); col++)
	{
		for (size_t row = 0; row < m_Grid->GetHeight(); row++)
		{
			FVector pos(origin.X + col * BLOCK_SIZE, origin.Y + row * BLOCK_SIZE, 0);
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

void ALevelGenerator::EmptyRandomTile()
{
	vector<Tile*> tiles = m_Grid->GetTiles();
	int randomNr = rand() % tiles.size();
	tiles[randomNr]->IsFilled = false;
	UpdateBlock(tiles[randomNr]);
}

void ALevelGenerator::UpdateBlock(Tile * tile)
{
	FVector origin(-m_Grid->GetWidth() / 2 * BLOCK_SIZE, -m_Grid->GetHeight() / 2 * BLOCK_SIZE, 0);

	UStaticMeshComponent* block = *m_BlockOutMap.Find(tile);
	if (tile->IsFilled)
		block->SetWorldLocation(FVector(origin.X + tile->X * BLOCK_SIZE, origin.Y + tile->Y * BLOCK_SIZE, -100));
	else
		block->SetWorldLocation(FVector(origin.X + tile->X * BLOCK_SIZE, origin.Y + tile->Y * BLOCK_SIZE, 0));
}