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


protected:
	int Width, Height;
	TileArr2D Tiles;
};