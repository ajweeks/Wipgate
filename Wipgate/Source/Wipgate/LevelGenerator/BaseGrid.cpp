#include "BaseGrid.h"
#include "Wipgate.h"
#include "GeneralFunctionLibrary_CPP.h"
#include <algorithm>
#include <functional>

BaseGrid::BaseGrid(const BaseGrid & other)
{
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_Tiles = other.m_Tiles;
}

BaseGrid::BaseGrid(const int width, const int height, UObject* outer)
	: m_Width(width), m_Height(height), m_Outer(outer)
{
	for (size_t col = 0; col < width; col++)
	{
		m_Tiles.push_back(vector<Tile*>());
		for (size_t row = 0; row < height; row++)
			m_Tiles[col].push_back(new Tile(FVector2D(col, row)));
	}
}

BaseGrid::BaseGrid(TileArr2D tiles, UObject* outer)
	: m_Outer(outer)
{
	m_Width = tiles.size();
	if (tiles.size() == 0) return;
	
	// calculate height based on the longest vector
	// this way width and height contain the outer limits of the 2D space in case it's not a rectangle (streets/open spaces)
	int maxHeight = 0;
	for (auto col : tiles)
	{
		if (col.size() > maxHeight)
			maxHeight = col.size();
	}
	m_Height = maxHeight;

	m_Tiles = tiles;
}

BaseGrid::~BaseGrid()
{
	m_Tiles.clear();
}

BaseGrid & BaseGrid::operator=(const BaseGrid & other)
{
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_Tiles = other.m_Tiles;
	return *this;
}

vector<Tile*> BaseGrid::GetTiles()
{
	vector<Tile*> tiles;
	for (auto col : m_Tiles)
	{
		for (auto tile : col)
			tiles.push_back(tile);
	}
	return tiles;
}

Tile * BaseGrid::GetRandomTile()
{
	vector<Tile*> tiles = GetTiles();
	if (tiles.empty()) return nullptr;
	return tiles[rand() % tiles.size()];
}

Tile * BaseGrid::GetRandomTileWithType(const TileType type)
{
	vector<Tile*> tilesWithType = GetTilesWithType(GetTiles(), type);
	if (tilesWithType.empty()) return nullptr;
	return tilesWithType[rand() % tilesWithType.size()];
}

void BaseGrid::SetFilled(Tile * tile, bool isFilled)
{
	tile->IsFilled = isFilled;
	AddStep_Fill(tile, isFilled);
}

void BaseGrid::SetFilledArea(const int bottom, const int left, const int top, const int right, bool isFilled)
{
	if (top < bottom || top >= m_Height || bottom < 0 || right < left || right >= m_Width || left < 0) {
		UE_LOG(LevelGenerator, Log, TEXT("BaseGrid::SetFilledArea || Dimensions are out of bounds!"));
		return;
	}

	for (size_t col = 0; col < m_Width; col++)
	{
		for (size_t row = 0; row < m_Height; row++)
		{
			if (row >= bottom && row <= top && col >= left && col <= right)
			{
				if (isFilled)
					m_Tiles[col][row]->SetType(WALL);
				SetFilled(m_Tiles[col][row], isFilled);
			}
		}
	}
}

void BaseGrid::SetFilledTiles(TileArr2D & tiles, const bool isFilled)
{
	int width = tiles.size();
	if (width == 0) {
		UE_LOG(LevelGenerator, Error, TEXT("SetFilledSet:: tiles size is 0"));
		return;
	}

	int height = tiles[0].size();
	for (size_t col = 0; col < width; col++)
	{
		for (size_t row = 0; row < height; row++)
			tiles[col][row]->IsFilled = isFilled;
	}
}

void BaseGrid::SetFilledTiles(vector<Tile*> tiles, const bool isFilled)
{
	for (auto t : tiles)
	{
		if (t != nullptr)
		{
			t->IsFilled = isFilled;
			AddStep_Fill(t, isFilled);
		}
	}
}

void BaseGrid::SetType(Tile * tile, const TileType type)
{
	tile->SetType(type);
	AddStep_Type(tile, type);
}

void BaseGrid::SetTilesType(vector<Tile*> tiles, const TileType type)
{ 
	for (auto t : tiles)
	{
		t->SetType(type);
		AddStep_Type(t, type);
	}
}

void BaseGrid::SetTileTypeArea(const int bottom, const int left, const int top, const int right, const TileType type)
{
	if (top < bottom || top >= m_Height || bottom < 0 || right < left || right >= m_Width || left < 0) {
		UE_LOG(LevelGenerator, Error, TEXT("BaseGrid::SetTileTypeArea || Dimensions are out of bounds!"));
		return;
	}

	for (size_t col = 0; col < m_Width; col++)
	{
		for (size_t row = 0; row < m_Height; row++)
		{
			if (row >= bottom && row <= top && col >= left && col <= right)
			{
				m_Tiles[col][row]->SetType(type);
				AddStep_Type(m_Tiles[col][row], type);
			}
		}
	}
}

void BaseGrid::SetTilesRegion(vector<Tile*> tiles, const TileRegion region)
{
	for (auto t : tiles)
	{
		t->SetRegion(region);
		// TODO: step
	}
}

void BaseGrid::SetTileRegionArea(const int bottom, const int left, const int top, const int right, const TileRegion region)
{
	if (top < bottom || top >= m_Height || bottom < 0 || right < left || right >= m_Width || left < 0) {
		UE_LOG(LevelGenerator, Error, TEXT("BaseGrid::SetTileRegionArea || Dimensions are out of bounds!"));
		return;
	}

	for (size_t col = 0; col < m_Width; col++)
	{
		for (size_t row = 0; row < m_Height; row++)
		{
			if (row >= bottom && row <= top && col >= left && col <= right)
				m_Tiles[col][row]->SetRegion(region);
		}
	}
}

vector<Tile*> BaseGrid::GetTilesFilled(const vector<Tile*> tiles, const bool isFilled)
{
	vector<Tile*> tilesFilled;
	for (auto t : tiles)
	{
		if (t->IsFilled == isFilled)
			tilesFilled.push_back(t);
	}
	return tilesFilled;
}

vector<Tile*> BaseGrid::GetTilesWithType(const vector<Tile*> tiles, const TileType type)
{
	vector<Tile*> tilesWithType;
	for (auto t : tiles)
	{
		if (t->GetType() == type)
			tilesWithType.push_back(t);
	}
	return tilesWithType;
}

vector<Tile*> BaseGrid::GetTilesWithTypes(const vector<Tile*> tiles, const vector<TileType> types)
{
	vector<Tile*> tilesWithType;
	for (auto t : tiles)
	{
		for (auto type : types)
		{
			if (t->GetType() == type)
			{
				tilesWithType.push_back(t);
				break;
			}
		}
	}
	return tilesWithType;
}

vector<Tile*> BaseGrid::GetTilesWithRegion(const vector<Tile*> tiles, const TileRegion region)
{
	vector<Tile*> tilesWithRegion;
	for (auto t : tiles)
	{
		if (t->GetRegion() == region)
			tilesWithRegion.push_back(t);
	}
	return tilesWithRegion;
}

vector<Tile*> BaseGrid::GetTilesWithRegions(const vector<Tile*> tiles, const vector<TileRegion> regions)
{
	vector<Tile*> tilesWithRegion;
	for (auto t : tiles)
	{
		for (auto region : regions)
		{
			if (t->GetRegion() == region)
			{
				tilesWithRegion.push_back(t);
				break;
			}
		}
	}
	return tilesWithRegion;
}

vector<Tile*> BaseGrid::GetTilesExcept(vector<Tile*> tiles, Tile * ex)
{
	for (auto t : tiles)
	{
		auto iter = find(tiles.begin(), tiles.end(), ex);
		if (iter != tiles.end())
			tiles.erase(iter);
	}
	return tiles;
}

bool BaseGrid::IsAdjTileWithType(const Tile * tile, const TileType type)
{
	for (auto adj : GetAdjacentTiles(tile))
	{
		if (m_Tiles[adj->X][adj->Y]->GetType() == type)
			return true;
	}
	return false;
}

bool BaseGrid::IsNearTileWithType(const Tile * tile, const TileType type)
{
	for (auto near : GetNearbyTiles(tile))
	{
		if (m_Tiles[near->X][near->Y]->GetType() == type)
			return true;
	}
	return false;
}

vector<Tile*> BaseGrid::GetAdjacentTiles(const Tile * tile)
{
		vector<Tile*> adjacentTiles;
	if (tile->X > 0 && tile->X < m_Width)
		adjacentTiles.push_back(GetLeftTile(tile));
	if (tile->X < m_Width - 1 && tile->X >= 0)
		adjacentTiles.push_back(GetRightTile(tile));
	if (tile->Y > 0 && tile->Y < m_Height)
		adjacentTiles.push_back(GetBottomTile(tile));
	if (tile->Y < m_Height - 1 && tile->Y >=0)
		adjacentTiles.push_back(GetTopTile(tile));
	return adjacentTiles;
}

vector<Tile*> BaseGrid::GetNearbyTiles(const Tile * tile)
{
	return GetNearbyTiles(tile->GetPosition());
}

vector<Tile*> BaseGrid::GetNearbyTiles(const FVector2D pos)
{
	vector<Tile*> nearbyTiles;
	FVector2D surPos;
	for (size_t i = 0; i < 9; i++)
	{
		surPos.X = pos.X + (i % 3) - 1;
		surPos.Y = pos.Y + (i / 3) - 1;
		if (surPos != pos && IsWithinBounds(surPos, FString("NULL")))
			nearbyTiles.push_back(m_Tiles[surPos.X][surPos.Y]);
	}
	return nearbyTiles;
}

Tile * BaseGrid::GetLeftTile(const Tile * t)
{
	if (!t) return nullptr;
	if (t->X <= 0) {
		UE_LOG(LevelGenerator, Warning, TEXT("BaseGrid::GetLeftTile || Out of bounds."));
		return nullptr;
	}
	return m_Tiles[t->X - 1][t->Y];
}

Tile * BaseGrid::GetRightTile(const Tile * t)
{
	if (!t) return nullptr;
	if (t->X >= m_Width - 1) {
		UE_LOG(LevelGenerator, Warning, TEXT("BaseGrid::GetRightTile || Out of bounds."));
		return nullptr;
	}
	return m_Tiles[t->X + 1][t->Y];
}

Tile * BaseGrid::GetTopTile(const Tile * t)
{
	if (!t) return nullptr;
	if (t->Y >= m_Height - 1) {
		UE_LOG(LevelGenerator, Warning, TEXT("BaseGrid::GetTopTile || Out of bounds."));
		return nullptr;
	}
	return m_Tiles[t->X][t->Y + 1];
}

Tile * BaseGrid::GetBottomTile(const Tile * t)
{
	if (!t) return nullptr;
	if (t->Y <= 0) {
		UE_LOG(LevelGenerator, Warning, TEXT("BaseGrid::GetBottomTile || Out of bounds."));
		return nullptr;
	}
	return m_Tiles[t->X][t->Y - 1];
}

vector<Tile*> BaseGrid::GetOppositeDirTiles(const Tile * tile, const Direction dir)
{
	vector<Tile*> opposites;
	Tile* temp = new Tile();
	switch (dir)
	{
	case TOP:
		temp = GetTileAt(tile, BOTTOM);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, BOTTOM_RIGHT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, BOTTOM_LEFT);
		if (temp) opposites.push_back(temp);
		break;
	case TOP_RIGHT:
		temp = GetTileAt(tile, BOTTOM_LEFT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, BOTTOM);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, LEFT);
		if (temp) opposites.push_back(temp);
		break;
	case RIGHT:
		temp = GetTileAt(tile, LEFT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, BOTTOM_LEFT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, TOP_LEFT);
		if (temp) opposites.push_back(temp);
		break;
	case BOTTOM_RIGHT:
		temp = GetTileAt(tile, TOP_LEFT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, LEFT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, TOP);
		if (temp) opposites.push_back(temp);
		break;
	case BOTTOM:
		temp = GetTileAt(tile, TOP);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, TOP_LEFT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, TOP_RIGHT);
		if (temp) opposites.push_back(temp);
		break;
	case BOTTOM_LEFT:
		temp = GetTileAt(tile, TOP_RIGHT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, RIGHT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, TOP);
		if (temp) opposites.push_back(temp);
		break;
	case LEFT:
		temp = GetTileAt(tile, RIGHT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, TOP_RIGHT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, BOTTOM_RIGHT);
		if (temp) opposites.push_back(temp);
		break;
	case TOP_LEFT:
		temp = GetTileAt(tile, BOTTOM_RIGHT);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, BOTTOM);
		if (temp) opposites.push_back(temp);
		temp = GetTileAt(tile, RIGHT);
		if (temp) opposites.push_back(temp);
		break;
	case NO_DIRECTION:
		UE_LOG(LevelGenerator, Warning, TEXT("Opposite direction is NO_DIRECTION"));
	default:
		break;
	}
	return opposites;
}

vector<Tile*> BaseGrid::GetSubCol(const int col, const int rowStart, const int rowEnd)
{
	vector<Tile*> subCol;
	for (size_t i = rowStart; i < rowEnd; i++)
		subCol.push_back(m_Tiles[col][i]);
	return subCol;
}

vector<Tile*> BaseGrid::GetSubRow(const int row, const int colStart, const int colEnd)
{
	vector<Tile*> subCol;
	for (size_t i = colStart; i < colEnd; i++)
		subCol.push_back(m_Tiles[i][row]);
	return subCol;
}

TileArr2D BaseGrid::TilesTo2D(vector<Tile*> tiles)
{
	TileArr2D tiles2D = TileArr2D();
	sort(tiles.begin(), tiles.end(), TileCompare());

	int previousX = tiles[0]->X;
	tiles2D.push_back(vector<Tile*>());
	int i = 0;
	int col = 0;
	while (i < tiles.size())
	{
		if (tiles[i]->X != previousX)
		{
			tiles2D.push_back(vector<Tile*>());
			col++;
		}
		tiles2D[col].push_back(tiles[i]);

		previousX = tiles[i]->X;
		i++;
	}
	//LogTileCoordinates(tiles2D);
	return tiles2D;
}

void BaseGrid::PrintDirectionToScreen(const Direction dir)
{
	switch (dir)
	{
	case TOP:
		PrintStringToScreen("Top");
		break;
	case TOP_RIGHT:
		PrintStringToScreen("Top right");
		break;
	case RIGHT:
		PrintStringToScreen("Right");
		break;
	case BOTTOM_RIGHT:
		PrintStringToScreen("Bottom right");
		break;
	case BOTTOM:
		PrintStringToScreen("Bottom");
		break;
	case BOTTOM_LEFT:
		PrintStringToScreen("Bottom left");
		break;
	case LEFT:
		PrintStringToScreen("Left");
		break;
	case TOP_LEFT:
		PrintStringToScreen("Top left");
		break;
	case NO_DIRECTION:
		PrintStringToScreen("No direction");
		break;
	default:
		break;
	}
}

void BaseGrid::LogDirection(const Direction dir)
{
	switch (dir)
	{
	case TOP:
		UE_LOG(LevelGenerator, Log, TEXT("Top"));
		break;
	case TOP_RIGHT:
		UE_LOG(LevelGenerator, Log, TEXT("Top right"));
		break;
	case RIGHT:
		UE_LOG(LevelGenerator, Log, TEXT("Right"));
		break;
	case BOTTOM_RIGHT:
		UE_LOG(LevelGenerator, Log, TEXT("Bottom right"));
		break;
	case BOTTOM:
		UE_LOG(LevelGenerator, Log, TEXT("Bottom"));
		break;
	case BOTTOM_LEFT:
		UE_LOG(LevelGenerator, Log, TEXT("Bottom left"));
		break;
	case LEFT:
		UE_LOG(LevelGenerator, Log, TEXT("Left"));
		break;
	case TOP_LEFT:
		UE_LOG(LevelGenerator, Log, TEXT("Top left"));
		break;
	case NO_DIRECTION:
		UE_LOG(LevelGenerator, Log, TEXT("No direction"));
		break;
	default:
		break;
	}
}

vector<Tile*> BaseGrid::GetFloodType(Tile * start, const TileType type)
{
	TQueue<Tile*> nodeQueue;
	vector<Tile*> explored;
	nodeQueue.Enqueue(start);

	while (!nodeQueue.IsEmpty())
	{
		Tile* current;
		nodeQueue.Dequeue(current);

		vector<Tile*> adjacents = GetAdjacentTiles(current);
		adjacents = GetTilesWithType(adjacents, type);

		for (auto node : adjacents)
		{
			// make sure no explored nodes are checked again
			if (std::find(explored.begin(), explored.end(), node) != explored.end())
				continue;
			// mark node as explored
			explored.push_back(node);
			// add to queue of nodes to examine
			nodeQueue.Enqueue(node);
		}
	}

	return explored;
}

Direction BaseGrid::GetTileDirection(const Tile * from, const Tile * to)
{
	for (int dirIndex = TOP; dirIndex != NO_DIRECTION; dirIndex++)
	{
		Direction dir = static_cast<Direction>(dirIndex);
		if (to == GetTileAt(from, dir))
			return dir;
	}
	return NO_DIRECTION;
}

Tile * BaseGrid::GetTileAt(const Tile* t, const Direction dir)
{
	switch (dir)
	{
	case TOP:
		return GetTopTile(t);
	case TOP_RIGHT:
		return GetRightTile(GetTopTile(t));
	case RIGHT:
		return GetRightTile(t);
	case BOTTOM_RIGHT:
		return GetRightTile(GetBottomTile(t));
	case BOTTOM:
		return GetBottomTile(t);
	case BOTTOM_LEFT:
		return GetLeftTile(GetBottomTile(t));
	case LEFT:
		return GetLeftTile(t);
	case TOP_LEFT:
		return GetLeftTile(GetTopTile(t));
	default:
		return nullptr;
		break;
	}
}

void BaseGrid::AddStep_Fill(Tile * tile, const bool isFilled)
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

void BaseGrid::AddStep_Type(Tile * tile, const TileType type)
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

bool BaseGrid::IsWithinBounds(const FVector2D pos, const FString logInfo)
{
	if (pos.X >= 0 && pos.X < m_Width &&
		pos.Y >= 0 && pos.Y < m_Height)
		return true;

	if (logInfo != "NULL")
		UE_LOG(LevelGenerator, Warning, TEXT("%s || Position[%f,%f] out of grid bounds."), *logInfo, pos.X, pos.Y);
	return false;
}

bool BaseGrid::IsWithinBounds(const int x, const int y, const FString logInfo)
{
	return IsWithinBounds(FVector2D(x, y), logInfo);
}

bool BaseGrid::IsWithinBounds(const Tile* tile, const FString logInfo)
{
	return IsWithinBounds(FVector2D(tile->X, tile->Y), logInfo);
}

void BaseGrid::LogTileCoordinates(TileArr2D tiles)
{
	FString log;
	log += "GRID COORDINATES [2D]: \n";

	for (auto col : tiles)
	{
		for (auto t : col)
		{
			if (t)
			{
				log += "[";
				log.AppendInt(t->X);
				log += ", ";
				log.AppendInt(t->Y);
				log += "]";
			}
		}
		log += "\n";
	}

	UE_LOG(LevelGenerator, Log, TEXT("%s"), *log);
}

void BaseGrid::LogTileCoordinates(vector<Tile*> tiles)
{
	FString log;
	log += "GRID COORDINATES [1D]: \n";

	for (auto t : tiles)
	{
		if (t)
		{
			log += "[";
			log.AppendInt(t->X);
			log += ", ";
			log.AppendInt(t->Y);
			log += "]";
		}
	}

	UE_LOG(LevelGenerator, Log, TEXT("%s"), *log);
}
