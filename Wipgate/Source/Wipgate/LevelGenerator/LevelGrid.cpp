#include "LevelGrid.h"
#include "GeneralFunctionLibrary_CPP.h"
#include "Wipgate.h"
#include <algorithm>
#include "Tile.h"

LevelGrid::LevelGrid(const LevelGrid & other)
	: BaseGrid(other)
{
	m_Parent = other.m_Parent;
	m_Children = other.m_Children;
}

LevelGrid & LevelGrid::operator=(const LevelGrid & other)
{
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_Tiles = other.m_Tiles;
	m_MainGrid = other.m_MainGrid;
	m_Parent = other.m_Parent;
	m_Children = other.m_Children;
	return *this;
}

vector<LevelGrid*> LevelGrid::GetChildrenDeep()
{
	vector<LevelGrid*> children;
	// if this grid has children -> look for more children
	if (m_Children.size() != 0)
	{
		for (auto c : m_Children)
		{
			for (auto cc : c->GetChildrenDeep())
			{
				children.push_back(cc);
			}
		}
	}
	// else add this child
	else
		children.push_back(this);

	return children;
}

LevelGrid * LevelGrid::CreateSubGrid(const int bottom, const int left, const int top, const int right)
{
	vector<vector<Tile*>> subgTiles;
	for (size_t col = 0; col < m_Width; col++)
	{
		if (col >= left && col <= right)
			subgTiles.push_back(vector<Tile*>());
		for (size_t row = 0; row < m_Height; row++)
		{
			// Add the tiles inside the specified rect to the sub-grid tileset
			if (row >= bottom && row <= top && col >= left && col <= right)
			{
				if (IsWithinBounds(col, row, FString("LevelGrid::CreateSubGrid")))
					subgTiles[col - left].push_back(m_Tiles[col][row]);
			}
		}
	}

	LevelGrid* newGrid = new LevelGrid(subgTiles, m_Outer);
	// TODO: Set subgrid parameters based on this grids parameters
	newGrid->SetMainGrid(m_MainGrid);
	
	return newGrid;
}

bool LevelGrid::Split(const int sizeMin)
{
	/*	50% chance to split vertically or horizontally
	split grid in 2, taking into account the minimum size */
	LevelGrid *subLeft(new LevelGrid(*this)), *subRight(new LevelGrid(*this));
	bool hasSplit = false;

	int rnd = rand() % 2;
	// First try to split based on the random dice roll
	if (rnd == 0)
		hasSplit = SplitHorizontal(sizeMin, *subLeft, *subRight);
	else
		hasSplit = SplitVertical(sizeMin, *subLeft, *subRight);
	// If that fails try to split in the other direction, taking into account the minimum size
	if (!hasSplit && m_Width / 2 >= sizeMin)
		hasSplit = SplitHorizontal(sizeMin, *subLeft, *subRight);
	else if (!hasSplit && m_Height / 2 >= sizeMin)
		hasSplit = SplitVertical(sizeMin, *subLeft, *subRight);

	if (!hasSplit)
		return false;

	AddChild(subLeft);
	AddChild(subRight);
	return true;
}

bool LevelGrid::SplitDeep(const int sizeMin, int level)
{
	// Deep split child grids if the current split was successful
	if (Split(sizeMin + rand() % 4))
	{
		for (auto c : m_Children)
			c->SplitDeep(sizeMin, level++);
		return true;
	}

	return false;
}

bool LevelGrid::SplitHorizontal(const int sizeMin, LevelGrid & subLeft, LevelGrid & subRight)
{
	if (m_Width / 2 < sizeMin) {
		//UE_LOG(LogTemp, Warning, TEXT("LevelGrid::Split || Grid is not wide enough to split"));
		return false;
	}
	int x = rand() % (m_Width - 2 * sizeMin + 1) + sizeMin;
	// - 1 to prevent grid overlap
	subLeft = *CreateSubGrid(0, 0, m_Height, x - 1);
	subRight = *CreateSubGrid(0, x, m_Height, m_Width);
	return true;
}

bool LevelGrid::SplitVertical(const int sizeMin, LevelGrid& subLeft, LevelGrid& subRight)
{
	if (m_Height / 2 < sizeMin) {
		//UE_LOG(LogTemp, Warning, TEXT("LevelGrid::Split || Grid is not high enough to split [420 blaze it]"));
		return false;
	}

	int y = rand() % (m_Height - 2 * sizeMin + 1) + sizeMin;
	subLeft = *CreateSubGrid(0, 0, y - 1, m_Width);
	subRight = *CreateSubGrid(y, 0, m_Height, m_Width);
	return true;
}

void LevelGrid::AddStreet(vector<Tile*> tiles)
{
	m_Streets.push_back(new Street(TilesTo2D(tiles), m_Outer, m_MainGrid));
}

void LevelGrid::RemoveStreet(Street * street)
{
	SetFilledTiles(street->GetTiles(), true);
	auto it = find(m_Streets.begin(), m_Streets.end(), street);
	m_Streets.erase(it);
}

vector<Street*> LevelGrid::GetEdgeStreets()
{
	vector<Street*> edgeStreets;
	for (auto street : m_Streets)
	{
		if (street->IsEdgeStreet())
			edgeStreets.push_back(street);
	}
	return edgeStreets;
}

vector<Tile*> LevelGrid::FindPathWithoutStreets(Tile* start, Tile* goal, const vector<Street*> illegalStreets)
{
	TQueue<Tile*> nodeQueue;
	vector<Tile*> explored;
	TMap<Tile*, Tile*> connections;
	nodeQueue.Enqueue(start);

	while (!nodeQueue.IsEmpty())
	{
		Tile* current;
		nodeQueue.Dequeue(current);
		if (current == goal)
		{
			vector<Tile*> path;
			while (connections[current] != start)
			{
				current = connections[current];
				path.push_back(current);
			}
			return path;
		}

		vector<Tile*> adjacents = GetAdjacentTiles(current);
		adjacents = GetTilesFilled(adjacents, false);
		adjacents = GetTilesFromOtherStreets(adjacents, illegalStreets);

		for (auto node : adjacents)
		{
			// make sure no explored nodes are checked again
			if (std::find(explored.begin(), explored.end(), node) != explored.end())
				continue;

			// mark node as explored
			explored.push_back(node);
			// store reference to previous node
			connections.Add(node, current);
			// add to queue of nodes to examine
			nodeQueue.Enqueue(node);
		}

	}

	// return empty
	return vector<Tile*>();
}

vector<Tile*> LevelGrid::GetTilesFromOtherStreets(const vector<Tile*> tiles, const vector<Street*> illegalStreets)
{
	vector<Tile*> otherTiles = tiles;
	vector<Tile*> illegalTiles;

	for (auto t : tiles)
	{
		for (auto street : illegalStreets)
		{
			if (street == t->Street)
				illegalTiles.push_back(t);
		}
	}

	//otherTiles.erase(remove_if(begin(otherTiles), end(otherTiles),
	//	[&](auto x) {return find(begin(illegalTiles), end(illegalTiles), x) != end(illegalTiles); }), end(illegalTiles));

	for (int i = 0; i < illegalTiles.size(); i++)
	{
		auto iter = find(otherTiles.begin(), otherTiles.end(), illegalTiles[i]);
		if (iter != otherTiles.end())
			otherTiles.erase(iter);
	}

	return otherTiles;
}
