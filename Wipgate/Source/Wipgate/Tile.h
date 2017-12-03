#pragma once

#include "CoreMinimal.h"

enum TileType
{
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
	FVector2D GetPosition() { return FVector2D(X, Y); }

	/* --- Members --- */
	int X = -1;
	int Y = -1;

	bool IsFilled = true;
	TileType Type;
	TileRegion Region;
	FColor Color = FColor::Black;

	// TODO: parent to lowest subgrid (street, block or intersection)
};