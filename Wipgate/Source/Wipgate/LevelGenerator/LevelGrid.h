#pragma once
#include "BaseGrid.h"
#include <vector>

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
	void SetMainGrid(LevelGrid* main) { m_MainGrid = main; }
	void SetParentGrid(LevelGrid* parent) { m_Parent = parent; }

	LevelGrid* GetMainGrid() { return m_MainGrid; }
	LevelGrid* GetParent() { return m_Parent; }
	vector<LevelGrid*> GetChildren() { return m_Children; }
	vector<LevelGrid*> GetChildrenDeep();

	/* --- Subdivision --- */
	LevelGrid * CreateSubGrid(const int bottom, const int left, const int top, const int right);
	bool Split(const int sizeMin);
	bool SplitDeep(const int sizeMin, int level = 0);
	bool SplitHorizontal(const int sizeMin, LevelGrid& subLeft, LevelGrid& subRight);
	bool SplitVertical(const int sizeMin, LevelGrid& subLeft, LevelGrid& subRight);

	void AddChild(LevelGrid* child) { m_Children.push_back(child); }

	/* --- Steps --- */
	virtual void AddStep_Fill(Tile* tile, const bool isFilled) override;
	virtual void AddStep_Type(Tile* tile, const TileType type) override;

protected:
	LevelGrid* m_MainGrid;
	LevelGrid* m_Parent;
	vector<LevelGrid*> m_Children;
};

