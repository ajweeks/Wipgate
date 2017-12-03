#include "Tile.h"

Tile::Tile(const Tile & other)
{
	IsFilled = other.IsFilled;
	Color = other.Color;
	X = other.X;
	Y = other.Y;
	Type = other.Type;
//	Parent = other.Parent;
}