#pragma once

#include "CoreMinimal.h"
#include "BaseGrid.h"

typedef vector<vector<Tile*>> TileArr2D;

enum StreetOrientation
{
	HORIZONTAL,
	VERTICAL,
	NO_STREET_ORIENTATION,
};

class WIPGATE_API Street : public BaseGrid
{
public:
	Street(TileArr2D tiles, UObject* outer, BaseGrid* mainGrid);

	void Widen();
	void Tighten();
	bool IsEdgeStreet();
	vector<Tile*> GetIntersectionTilesFront();
	vector<Tile*> GetIntersectionTilesBack();

private:
	StreetOrientation m_Orientation = NO_STREET_ORIENTATION;
};
