#include "Street.h"
#include "Wipgate.h"

Street::Street(TileArr2D tiles, UObject* outer, BaseGrid* mainGrid)
	: BaseGrid(tiles, outer)
{
	m_MainGrid = mainGrid;
	TileType type;

	if (m_Width > m_Height) {
		m_Orientation = HORIZONTAL;
		type = STREET_HOR;
	}
	else
	{
		m_Orientation = VERTICAL;
		type = STREET_VERT;
	}

	for (auto col : m_Tiles)
	{
		for (auto tile : col)
		{
			SetType(tile, type);
			tile->Street = this;
		}
	}
}

void Street::Widen()
{
	vector<Tile*> newTiles;
	if (m_Orientation == HORIZONTAL)
	{
		if (m_MainGrid->IsWithinBounds(m_Tiles[0][0]->X , m_Tiles[0][0]->Y + m_Height))
		{
			newTiles = m_MainGrid->GetSubRow(m_Tiles[0][0]->Y + m_Height, m_Tiles[0][0]->X, m_Tiles[0][0]->X + m_Width);
			m_MainGrid->SetFilledTiles(newTiles, false);
			//LogTileCoordinates(m_Tiles);
			//LogTileCoordinates(newTiles);
		}
	}
	else
	{
		if (m_MainGrid->IsWithinBounds(m_Tiles[0][0]->X + m_Width, m_Tiles[0][0]->Y))
		{
			newTiles = m_MainGrid->GetSubCol(m_Tiles[0][0]->X + m_Width, m_Tiles[0][0]->Y, m_Tiles[0][0]->Y + m_Height);
			m_MainGrid->SetFilledTiles(newTiles, false);
			//LogTileCoordinates(m_Tiles);
			//LogTileCoordinates(newTiles);
		}
	}
}

void Street::Tighten()
{
	vector<Tile*> oldTiles;
	if (m_Orientation == HORIZONTAL)
		oldTiles = m_MainGrid->GetSubRow(m_Tiles[0][0]->Y, m_Tiles[0][0]->X, m_Tiles[0][0]->X + m_Width);
	else
		oldTiles = m_MainGrid->GetSubCol(m_Tiles[0][0]->X, m_Tiles[0][0]->Y, m_Tiles[0][0]->Y + m_Height);

	m_MainGrid->SetFilledTiles(oldTiles, true);
}

bool Street::IsEdgeStreet()
{
	Tile * topLeft = m_Tiles[0][0];
	Tile * bottomRight = m_Tiles[m_Width - 1].back();
	//SetType(topLeft, FLAGGED);
	//SetType(bottomRight, FLAGGED);

	vector<Tile*> adjacents = m_MainGrid->GetAdjacentTiles(topLeft);
	if (!m_MainGrid->GetTilesWithType(adjacents, LEVEL_EDGE).empty())
		return true;
	adjacents = m_MainGrid->GetAdjacentTiles(bottomRight);
	if (!m_MainGrid->GetTilesWithType(adjacents, LEVEL_EDGE).empty())
		return true;
	return false;
}

vector<Tile*> Street::GetIntersectionTilesFront()
{
	// TODO: add each connecting intersection tile
	vector<Tile*> intersectionTiles;
	if (m_Orientation == HORIZONTAL)
	{
		Tile* temp = m_MainGrid->GetLeftTile(m_Tiles[0][0]);
		if (temp && temp->GetType() == FLOOR)
			intersectionTiles.push_back(temp);
		//temp = m_MainGrid->GetLeftTile(m_Tiles[0][1]);
		//if (temp && temp->GetType() == FLOOR)
		//	intersectionTiles.push_back(temp);
	}
	else // vertical
	{
		Tile* temp = m_MainGrid->GetTopTile(m_Tiles[0][m_Height -1]);
		if (temp && temp->GetType() == FLOOR)
			intersectionTiles.push_back(temp);
		//temp = m_MainGrid->GetTopTile(m_Tiles[1][m_Height - 1]);
		//if (temp && temp->GetType() == FLOOR)
		//	intersectionTiles.push_back(temp);
	}
	return intersectionTiles;
}

vector<Tile*> Street::GetIntersectionTilesBack()
{
	// TODO: add each connecting intersection tile
	vector<Tile*> intersectionTiles;
	if (m_Orientation == HORIZONTAL)
	{
		Tile* temp = m_MainGrid->GetRightTile(m_Tiles[m_Width - 1][0]);
		if (temp && temp->GetType() == FLOOR)
			intersectionTiles.push_back(temp);
		//temp = m_MainGrid->GetRightTile(m_Tiles[m_Width - 1][1]);
		//if (temp && temp->GetType() == FLOOR)
		//	intersectionTiles.push_back(temp);
	}
	else // vertical
	{
		Tile* temp = m_MainGrid->GetBottomTile(m_Tiles[0][0]);
		if (temp && temp->GetType() == FLOOR)
			intersectionTiles.push_back(temp);
		//temp = m_MainGrid->GetBottomTile(m_Tiles[1][0]);
		//if (temp && temp->GetType() == FLOOR)
		//	intersectionTiles.push_back(temp);
	}
	return intersectionTiles;
}