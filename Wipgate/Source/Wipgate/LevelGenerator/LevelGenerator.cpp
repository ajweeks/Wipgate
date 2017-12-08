#include "LevelGenerator.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "GeneralFunctionLibrary_CPP.h"
#include "Wipgate.h"
#include "Step.h"

void ALevelGenerator::InitializeBlockout()
{
	m_Grid = new LevelGrid(35, 35, this);
	m_Grid->SetMainGrid(m_Grid);
	
	m_Blockout = GetWorld()->SpawnActor<ALevelBlockout>();
	m_Blockout->SetBlock(m_Block);
	m_Blockout->Initialize(m_Grid);

}

void ALevelGenerator::GenerateStreets(const int granularity)
{
	LevelGrid* grid = m_Grid->CreateSubGrid(2, 2, m_Grid->GetHeight() - 3, m_Grid->GetWidth() - 3);

	// empty grid
	//vector<Tile*> tiles = m_Grid->GetTiles();
	m_Grid->SetFilledArea(1, 1, m_Grid->GetHeight() - 2, m_Grid->GetWidth() - 2, false);
	m_Grid->SetTilesType(m_Grid->GetTiles(), LEVEL_EDGE);
	m_Grid->SetTileTypeArea(1, 1, m_Grid->GetHeight() - 2, m_Grid->GetWidth() - 2, FLOOR);
	ExecuteSteps();

	GenerateBaseLayout(grid, granularity);
	FlagStreets();
	CreateStreetsFromFlagged();

	// street sparsity
	//vector<Street*> edgeStreets = m_Grid->GetEdgeStreets();
	//for (auto street : edgeStreets)
	//{
	//	if (rand() % 3 == 0)
	//		m_Grid->RemoveStreet(street);
	//}

	/*for (auto street : m_Grid->GetStreets())
	{
		int rnd = rand() % 3;
		if (rnd == 0)
			street->Widen();
		else if (rnd == 1)
			street->Tighten();
	}*/
	ExecuteSteps();

	m_Grid->GetRandomStreet()->Widen();

	vector<Tile*> intersectionTiles = m_Grid->GetTilesWithType(m_Grid->GetTiles(), INTERSECTION);
	vector<Tile*> adjacents;
	for (auto t : intersectionTiles)
	{
		// adjacent to wall?
		adjacents = m_Grid->GetAdjacentTiles(t);
		if (!m_Grid->GetTilesFilled(adjacents).empty() && 
			(m_Grid->IsAdjTileWithType(t, STREET_HOR) || m_Grid->IsAdjTileWithType(t, STREET_VERT)))
		{
			Tile* filled = m_Grid->GetTilesFilled(adjacents)[0];
			vector<Tile*> adjEmpties = m_Grid->GetTilesFilled(m_Grid->GetAdjacentTiles(filled), false);
			if (adjEmpties.size() > 1 && !m_Grid->GetTilesWithTypes(adjEmpties, vector<TileType> { STREET_HOR, STREET_VERT}).empty())
			{
				m_Grid->SetFilled(filled, false);
				m_Grid->SetType(filled, INTERSECTION);
			}
		}
	}

	//Street* street = m_Grid->GetRandomStreet();
	//if (street)
	//{
	//	auto path = m_Grid->FindPathWithoutStreets(street->GetIntersectionTilesFront()[0],
	//		street->GetIntersectionTilesBack()[0], vector<Street*> {street});
	//	m_Grid->SetTilesType(path, FLAGGED);
	//}




	//// remove streets while ensuring connectivity
	//SparsifyStreetsRandom(4);

	//// Reset unfilled tiles to floor and flag streets again
	//m_Grid->SetTilesType(m_Grid->GetTilesFilled(m_Grid->GetTiles(), false), FLOOR);
	//FlagStreets();
	//CreateStreetsFromFlagged();

	m_Grid->SetFilled((*m_Grid)[0][0], false);
}

void ALevelGenerator::ExecuteStep(UStep * step)
{
	if(step)
		step->Execute(m_Blockout->GetBlockoutMap());
}

void ALevelGenerator::Reset()
{
	if (m_Blockout)
		m_Blockout->Destroy();
	if(m_Grid)
		delete m_Grid;
	InitializeBlockout();
}

void ALevelGenerator::ExecuteSteps()
{
	for (auto step : m_Grid->GetSteps())
		ExecuteStep(step);
	m_Grid->ClearSteps();
}

void ALevelGenerator::GenerateBaseLayout(LevelGrid* grid, const int granularity)
{
	grid->SplitDeep(granularity);
	auto children = grid->GetChildrenDeep();
	for (auto c : children)
	{
		c->SetFilledArea(1, 1, c->GetHeight() - 2, c->GetWidth() - 2, true);
	}
}

void ALevelGenerator::FlagStreets()
{
	vector<Tile*> tiles = m_Grid->GetTiles();
	vector<Tile*> emptyTiles = m_Grid->GetTilesFilled(tiles, false);

	// flag tiles diagonal from building corners
	for (auto t : emptyTiles)
	{
		vector<Tile*> nearby = m_Grid->GetNearbyTiles(t->GetPosition());
		vector<Tile*> nearbyFilleds = m_Grid->GetTilesFilled(nearby, true);
		if (nearbyFilleds.size() > 1)
			m_Grid->SetType(t, FLAGGED);
	}

	// complete street flagging by flagging intersections completely
	// flag the 3 opposite tiles from the building corner
	vector<Tile*> intersectionTiles = m_Grid->GetTilesWithType(tiles, FLOOR);
	m_Grid->SetTilesType(intersectionTiles, INTERSECTION);
	Tile* blockCorner;
	vector<Tile*> oppositeTiles;
	for (auto t : intersectionTiles)
	{
		// get nearby building block corner
		blockCorner = m_Grid->GetTilesFilled(m_Grid->GetNearbyTiles(t), true).front();
		Direction cornerDir = m_Grid->GetTileDirection(t, blockCorner);
		oppositeTiles = m_Grid->GetOppositeDirTiles(t, cornerDir);
		m_Grid->SetTilesType(oppositeTiles, INTERSECTION);
	}
}

void ALevelGenerator::CreateStreetsFromFlagged()
{
	Tile* start;
	vector<Tile*> streetTiles;
	start = m_Grid->GetRandomTileWithType(FLAGGED);

	while (start)
	{
		streetTiles = m_Grid->GetFloodType(start, FLAGGED);
		m_Grid->AddStreet(streetTiles);
		start = m_Grid->GetRandomTileWithType(FLAGGED);
	}
}

void ALevelGenerator::SparsifyStreetsRandom(const int odds)
{
	for (auto street : m_Grid->GetStreets())
	{
		if (rand() % odds == 0)
		{
			if (!street->GetIntersectionTilesFront().empty()
				&& !street->GetIntersectionTilesBack().empty())
			{
				Tile* front = street->GetIntersectionTilesFront()[0];
				Tile* back = street->GetIntersectionTilesBack()[0];
				if (front && back)
				{
					auto path = m_Grid->FindPathWithoutStreets(front, back, vector<Street*> {street});
					if (!path.empty())
					{
						m_Grid->SetTilesType(path, FLAGGED);
						m_Grid->RemoveStreet(street);
					}
				}
			}
		}
	}
}
