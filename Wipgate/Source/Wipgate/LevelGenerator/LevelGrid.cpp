#include "LevelGrid.h"
#include "GeneralFunctionLibrary_CPP.h"
#include "Wipgate.h"

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
	if (Split(sizeMin))
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

void LevelGrid::AddStep_Fill(Tile * tile, const bool isFilled)
{
	if (m_MainGrid)
	{
		if (m_MainGrid == this) {
			UStep_Fill* fillStep = NewObject<UStep_Fill>(m_Outer);
			fillStep->Initialize(tile, isFilled);
			m_Steps.Add(fillStep);
		}
		else
			m_MainGrid->AddStep_Fill(tile, isFilled);
	}
}

void LevelGrid::AddStep_Type(Tile * tile, const TileType type)
{
	if (m_MainGrid)
	{
		if (m_MainGrid == this) {
			UStep_Type* typeStep = NewObject<UStep_Type>(m_Outer);
			typeStep->Initialize(tile, type);
			m_Steps.Add(typeStep);
		}
		else 
			m_MainGrid->AddStep_Type(tile, type);
	}
}
