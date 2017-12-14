#pragma once

#include "CoreMinimal.h"

class Street;

enum TileType
{
	FLAGGED,
	FLOOR,
	WALL,
	STREET_HOR,
	STREET_VERT,
	BLOCK,
	INTERSECTION,
	LEVEL_EDGE,
};

enum TileRegion
{
	EDGE,
	CENTER,
	ENTRANCE,
};

struct WIPGATE_API Tile
{
public:
	Tile() {}
	Tile(FVector2D coordinates) {
		X = coordinates.X;
		Y = coordinates.Y;
	}
	Tile(bool isFilled) : IsFilled(isFilled) {}
	Tile(const Tile& other);
	~Tile() {}

	bool operator <(const Tile& other) const;

	/* --- Functions --- */
	FVector2D GetPosition() const { return FVector2D(X, Y); }
	TileType GetType() const { return m_Type; }
	TileRegion GetRegion() const { return m_Region; }
	void SetType(const TileType type); // Sets type and sets color accordingly
	void SetRegion(const TileRegion region); // Sets region and sets color accordingly

	/* --- Members --- */
	int X = -1;
	int Y = -1;
	bool IsFilled = true;
	FColor Color = FColor::Black;
	Street* Street;
	// TODO: parent to lowest subgrid (street, block or intersection)

private:
	TileType m_Type;
	TileRegion m_Region;

	const FColor COL_STREET_HOR = FColor(255, 235, 200);
	const FColor COL_STREET_VERT = FColor(235, 255, 200);
};

struct WIPGATE_API TileCompare
{
	bool operator() (const Tile * a, const Tile * b) const {
		if (a->X < b->X)
			return true;
		if (a->X > b->X)
			return false;
		if (a->Y < b->Y)
			return true;
		if (a->Y > b->Y)
			return false;
		return true;
	}
};