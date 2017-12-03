#include "BaseGrid.h"
#include "Wipgate.h"

BaseGrid::BaseGrid(const int width, const int height) :
	Width(width), Height(height)
{
	for (size_t col = 0; col < width; col++)
	{
		Tiles.push_back(vector<Tile*>());
		for (size_t row = 0; row < height; row++)
			Tiles[col].push_back(new Tile(FVector2D(col, row)));
	}
}

BaseGrid::BaseGrid(vector<vector<Tile*>>& tiles)
{
	Width = tiles.size();
	if (!tiles.empty()) {
		Height = tiles[0].size();
		return;
	}
	Tiles = tiles;
}

BaseGrid::~BaseGrid()
{
	Tiles.clear();
}

vector<Tile*> BaseGrid::GetTiles()
{
	vector<Tile*> tiles;
	for (auto col : Tiles)
	{
		for (auto tile : col)
			tiles.push_back(tile);
	}
	return tiles;
}

void BaseGrid::SetFilledArea(const int bottom, const int left, const int top, const int right, bool isFilled)
{
	if (top < bottom || top >= Height || bottom < 0 || right < left || right >= Width || left < 0) {
		UE_LOG(LevelGenerator, Log, TEXT("BaseGrid::SetFilledArea || Dimensions are out of bounds!"));
		return;
	}

	for (size_t col = 0; col < Width; col++)
	{
		for (size_t row = 0; row < Height; row++)
		{
			if (row >= bottom && row <= top && col >= left && col <= right)
				Tiles[col][row]->IsFilled = isFilled;
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
			t->IsFilled = isFilled;
	}
}

void BaseGrid::SetTileTypeArea(const int bottom, const int left, const int top, const int right, const TileType type)
{
	if (top < bottom || top >= Height || bottom < 0 || right < left || right >= Width || left < 0) {
		UE_LOG(LevelGenerator, Error, TEXT("BaseGrid::SetTileTypeArea || Dimensions are out of bounds!"));
		return;
	}

	for (size_t col = 0; col < Width; col++)
	{
		for (size_t row = 0; row < Height; row++)
		{
			if (row >= bottom && row <= top && col >= left && col <= right)
				Tiles[col][row]->Type = type;
		}
	}
}

void BaseGrid::SetTileRegionArea(const int bottom, const int left, const int top, const int right, const TileRegion region)
{
	if (top < bottom || top >= Height || bottom < 0 || right < left || right >= Width || left < 0) {
		UE_LOG(LevelGenerator, Error, TEXT("BaseGrid::SetTileRegionArea || Dimensions are out of bounds!"));
		return;
	}

	for (size_t col = 0; col < Width; col++)
	{
		for (size_t row = 0; row < Height; row++)
		{
			if (row >= bottom && row <= top && col >= left && col <= right)
				Tiles[col][row]->Region = region;
		}
	}
}

vector<Tile*> BaseGrid::GetTilesWithType(const vector<Tile*> tiles, const TileType type)
{
	vector<Tile*> tilesWithType;
	for (auto t : tiles)
	{
		if (t->Type == type)
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
			if (t->Type == type)
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
		if (t->Region == region)
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
			if (t->Region == region)
			{
				tilesWithRegion.push_back(t);
				break;
			}
		}
	}
	return tilesWithRegion;
}

bool BaseGrid::IsAdjTileWithType(const Tile * tile, const TileType type)
{
	for (auto adj : GetAdjacentTiles(tile))
	{
		if (Tiles[adj->X][adj->Y]->Type == type)
			return true;
	}
	return false;
}

bool BaseGrid::IsNearTileWithType(const Tile * tile, const TileType type)
{
	for (auto near : GetNearbyTiles(tile))
	{
		if (Tiles[near->X][near->Y]->Type == type)
			return true;
	}
	return false;
}

vector<Tile*> BaseGrid::GetAdjacentTiles(const Tile * tile)
{
		vector<Tile*> adjacentTiles;
	if (tile->X > 0 && tile->X < Width)
		adjacentTiles.push_back(GetLeftTile(tile));
	if (tile->X < Width - 1 && tile->X >= 0)
		adjacentTiles.push_back(GetRightTile(tile));
	if (tile->Y > 0 && tile->Y < Height)
		adjacentTiles.push_back(GetBottomTile(tile));
	if (tile->Y < Height - 1 && tile->Y >=0)
		adjacentTiles.push_back(GetTopTile(tile));
	return adjacentTiles;
}

vector<Tile*> BaseGrid::GetNearbyTiles(const Tile * tile)
{
	vector<Tile*> surroundingTiles;
	Tile* surTile = new Tile();
	for (size_t i = 0; i < 9; i++)
	{
		surTile->X = tile->X + (i % 3) - 1;
		surTile->Y = tile->Y + (i / 3) - 1;
		if (surTile != tile && IsWithinBounds(surTile, FString("NULL")))
			surroundingTiles.push_back(surTile);
	}
	return surroundingTiles;
}

Tile * BaseGrid::GetLeftTile(const Tile * t)
{
	if (t->X <= 0) {
		UE_LOG(LevelGenerator, Warning, TEXT("BaseGrid::GetLeftTile || Out of bounds."), );
		return nullptr;
	}
	return Tiles[t->X - 1][t->Y];
}

Tile * BaseGrid::GetRightTile(const Tile * t)
{
	if (t->X >= Width - 1) {
		UE_LOG(LevelGenerator, Warning, TEXT("BaseGrid::GetRightTile || Out of bounds."), );
		return nullptr;
	}
	return Tiles[t->X + 1][t->Y];
}

Tile * BaseGrid::GetTopTile(const Tile * t)
{
	if (t->Y >= Height - 1) {
		UE_LOG(LevelGenerator, Warning, TEXT("BaseGrid::GetTopTile || Out of bounds."), );
		return nullptr;
	}
	return Tiles[t->X][t->Y + 1];
}

Tile * BaseGrid::GetBottomTile(const Tile * t)
{
	if (t->Y <= 0) {
		UE_LOG(LevelGenerator, Warning, TEXT("BaseGrid::GetBottomTile || Out of bounds."), );
		return nullptr;
	}
	return Tiles[t->X][t->Y - 1];
}

bool BaseGrid::IsWithinBounds(const FVector2D pos, const FString logInfo)
{
	if (pos.X >= 0 && pos.X < Width &&
		pos.Y >= 0 && pos.Y < Height)
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


