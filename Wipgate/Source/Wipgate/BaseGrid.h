#pragma once

#include "CoreMinimal.h"
#include "Tile.h"
#include <vector>

using namespace std;

typedef vector<vector<Tile*>> TileArr2D;

class WIPGATE_API BaseGrid
{
public:
	BaseGrid(const int width, const int height);
	BaseGrid(vector<vector<Tile*>>& tiles);
	~BaseGrid();

	/* --- Getters --- */
	TileArr2D& GetTiles2D() { return Tiles; }
	vector<Tile*> GetTiles();
	vector<Tile*> operator[](const int col) { return Tiles[col]; }
	int GetWidth() const { return Width; }
	int GetHeight() const { return Height; }

	/* --- Fillers --- */
	void SetFilledArea(const int bottom, const int left, const int top, const int right, bool isFilled = false);
	void SetFilledTiles(TileArr2D& tiles, const bool isFilled = false);
	void SetFilledTiles(vector<Tile*> tiles, const bool isFilled = false);

	/* --- Types and Regions --- */
	void SetTileType(vector<Tile*> tiles, const TileType type) { for (auto t : tiles) t->Type = type; }
	void SetTileTypeArea(const int bottom, const int left, const int top, const int right, const TileType type);
	void SetTileRegion(vector<Tile*> tiles, const TileRegion region) { for (auto t : tiles) t->Region = region; }
	void SetTileRegionArea(const int bottom, const int left, const int top, const int right, const TileRegion region);

	/* --- State requirements --- */
	vector<Tile*> GetTilesWithType(const vector<Tile*> tiles, const TileType type);
	vector<Tile*> GetTilesWithTypes(const vector<Tile*> tiles, const vector<TileType> types);
	vector<Tile*> GetTilesWithRegion(const vector<Tile*> tiles, const TileRegion region);
	vector<Tile*> GetTilesWithRegions(const vector<Tile*> tiles, const vector<TileRegion> regions);

	bool IsAdjTileWithType(const Tile* tile, const TileType type);
	bool IsNearTileWithType(const Tile* tile, const TileType type);

	/* --- Positional requirements --- */
	vector<Tile*> GetAdjacentTiles(const Tile* tile);
	vector<Tile*> GetNearbyTiles(const Tile* tile);
	Tile * GetLeftTile(const Tile * t);
	Tile * GetRightTile(const Tile * t);
	Tile * GetTopTile(const Tile * t);
	Tile * GetBottomTile(const Tile * t);

	/* --- Safety --- */
	bool IsWithinBounds(const FVector2D pos, const FString logInfo = "");
	bool IsWithinBounds(const int x, const int y, const FString logInfo = "");
	bool IsWithinBounds(const Tile* tile, const FString logInfo = "");

protected:
	int Width, Height;
	TileArr2D Tiles;
};