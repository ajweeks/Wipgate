#pragma once

#include "CoreMinimal.h"

enum TileType
{
	FLOOR,
	WALL,
	STREET,
	BLOCK,
	INTERSECTION,
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
	// TODO: parent to lowest subgrid (street, block or intersection)

private:
	TileType m_Type;
	TileRegion m_Region;
};