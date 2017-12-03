#include "BaseGrid.h"

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
