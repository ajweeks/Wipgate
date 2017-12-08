#pragma once

#include "CoreMinimal.h"
#include "Tile.h"
#include <vector>
#include "Step.h"

using namespace std;

typedef vector<vector<Tile*>> TileArr2D;

enum Direction
{
	TOP,
	TOP_RIGHT,
	RIGHT,
	BOTTOM_RIGHT,
	BOTTOM,
	BOTTOM_LEFT,
	LEFT,
	TOP_LEFT,
	NO_DIRECTION,
};

class WIPGATE_API BaseGrid
{
public:
	BaseGrid(const BaseGrid& other);
	BaseGrid(const int width, const int height, UObject* outer);
	BaseGrid(TileArr2D tiles, UObject* outer);
	virtual ~BaseGrid();

	BaseGrid& operator =(const BaseGrid& other);

	/* --- Getters & Setters --- */
	TileArr2D& GetTiles2D() { return m_Tiles; }
	vector<Tile*> GetTiles();
	vector<Tile*> operator[](const int col) { return m_Tiles[col]; }
	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }

	void SetMainGrid(BaseGrid* main) { m_MainGrid = main; }
	void SetParentGrid(BaseGrid* parent) { m_Parent = parent; }
	BaseGrid* GetMainGrid() { return m_MainGrid; }
	BaseGrid* GetParent() { return m_Parent; }

	/* --- Random Getters --- */
	Tile* GetRandomTile();
	Tile* GetRandomTileWithType(const TileType type);

	/* --- Fillers --- */
	void SetFilled(Tile* tile, bool isFilled);
	void SetFilledArea(const int bottom, const int left, const int top, const int right, bool isFilled = false);
	void SetFilledTiles(TileArr2D& tiles, const bool isFilled = false);
	void SetFilledTiles(vector<Tile*> tiles, const bool isFilled = false);

	/* --- Types and Regions --- */
	void SetType(Tile* tile, const TileType type);
	void SetTilesType(vector<Tile*> tiles, const TileType type);
	void SetTileTypeArea(const int bottom, const int left, const int top, const int right, const TileType type);
	void SetTilesRegion(vector<Tile*> tiles, const TileRegion region);
	void SetTileRegionArea(const int bottom, const int left, const int top, const int right, const TileRegion region);

	/* --- State requirements --- */
	vector<Tile*> GetTilesFilled(const vector<Tile*> tiles, const bool isFilled = true);
	vector<Tile*> GetTilesWithType(const vector<Tile*> tiles, const TileType type);
	vector<Tile*> GetTilesWithTypes(const vector<Tile*> tiles, const vector<TileType> types);
	vector<Tile*> GetTilesWithRegion(const vector<Tile*> tiles, const TileRegion region);
	vector<Tile*> GetTilesWithRegions(const vector<Tile*> tiles, const vector<TileRegion> regions);
	vector<Tile*> GetTilesExcept(vector<Tile*> tiles, Tile* ex);

	bool IsAdjTileWithType(const Tile* tile, const TileType type);
	bool IsNearTileWithType(const Tile* tile, const TileType type);

	/* --- Positional requirements --- */
	vector<Tile*> GetAdjacentTiles(const Tile* tile);
	vector<Tile*> GetNearbyTiles(const Tile* tile);
	vector<Tile*> GetNearbyTiles(const FVector2D pos);

	Direction GetTileDirection(const Tile* from, const Tile* to);
	Tile* GetTileAt(const Tile* t, const Direction dir);
	Tile * GetLeftTile(const Tile * t);
	Tile * GetRightTile(const Tile * t);
	Tile * GetTopTile(const Tile * t);
	Tile * GetBottomTile(const Tile * t);
	vector<Tile*> GetOppositeDirTiles(const Tile* tile, const Direction dir);

	vector<Tile*> GetSubCol(const int col, const int rowStart, const int rowEnd);
	vector<Tile*> GetSubRow(const int row, const int colStart, const int colEnd);

	/* --- Utility --- */
	TileArr2D TilesTo2D(vector<Tile*> tiles);
	void PrintDirectionToScreen(const Direction dir);
	void LogDirection(const Direction dir);

	/* --- Floodfills --- */
	vector<Tile*> GetFloodType(Tile* start, const TileType type);

	/* --- Steps --- */
	virtual void AddStep_Fill(Tile* tile, const bool isFilled);
	virtual void AddStep_Type(Tile* tile, const TileType type);
	TArray<UStep*>& GetSteps() { return m_Steps; }
	void ClearSteps() { m_Steps.Empty(); }

	/* --- Safety --- */
	bool IsWithinBounds(const FVector2D pos, const FString logInfo = "");
	bool IsWithinBounds(const int x, const int y, const FString logInfo = "");
	bool IsWithinBounds(const Tile* tile, const FString logInfo = "");

	/* --- Logging --- */
	void LogTileCoordinates(TileArr2D tiles);
	void LogTileCoordinates(vector<Tile*> tiles);

protected:
	int m_Width, m_Height;
	TileArr2D m_Tiles;
	TArray<UStep*> m_Steps;
	UObject* m_Outer;

	BaseGrid* m_MainGrid;
	BaseGrid* m_Parent;
};