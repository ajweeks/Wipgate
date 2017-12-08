#pragma once
#include "BaseGrid.h"
#include <vector>
#include "Street.h"

typedef vector<vector<Tile*>> TileArr2D;

class WIPGATE_API LevelGrid: public BaseGrid
{
public:
	LevelGrid(const LevelGrid & other);
	LevelGrid(const int width, const int height, UObject* outer) : BaseGrid(width, height, outer) {}
	LevelGrid(TileArr2D& tiles, UObject* outer) : BaseGrid(tiles, outer) {}
	virtual ~LevelGrid() {}

	LevelGrid& operator =(const LevelGrid& other);

	/* --- Setters & Getters --- */
	vector<LevelGrid*> GetChildren() { return m_Children; }
	vector<LevelGrid*> GetChildrenDeep();

	vector<Street*> GetStreets() { return m_Streets; }
	Street* GetRandomStreet() { return m_Streets[rand() % m_Streets.size()]; }

	/* --- Subdivision --- */
	LevelGrid * CreateSubGrid(const int bottom, const int left, const int top, const int right);
	bool Split(const int sizeMin);
	bool SplitDeep(const int sizeMin, int level = 0);
	bool SplitHorizontal(const int sizeMin, LevelGrid& subLeft, LevelGrid& subRight);
	bool SplitVertical(const int sizeMin, LevelGrid& subLeft, LevelGrid& subRight);

	void AddChild(LevelGrid* child) { m_Children.push_back(child); }

	/* --- Streets --- */
	void AddStreet(vector<Tile*> tiles);
	void RemoveStreet(Street* street);
	vector<Street*> GetEdgeStreets();
	vector<Tile*> FindPathWithoutStreets(Tile* start, Tile* goal, const vector<Street*> illegalStreets);

	vector<Tile*> GetTilesFromOtherStreets(const vector<Tile*> tiles, const vector<Street*> illegalStreets);

private:
	vector<LevelGrid*> m_Children;
	vector<Street*> m_Streets;
};