#include "Tile.h"
#include "Wipgate.h"

Tile::Tile(const Tile & other)
{
	IsFilled = other.IsFilled;
	Color = other.Color;
	X = other.X;
	Y = other.Y;
	m_Type = other.m_Type;
	m_Region = other.m_Region;
//	Parent = other.Parent;
}

bool Tile::operator <(const Tile& other) const
{
	if (X < other.X)
		return true;
	if (X > other.X)
		return false;
	if (Y < other.Y)
		return true;
	if (Y > other.Y)
		return false;
	return true;
}

void Tile::SetType(const TileType type)
{
	m_Type = type;
	switch (type)
	{
	case FLAGGED:
		Color = FColor::Red;
		break;
	case FLOOR:
		Color = FColor::White;
		break;
	case WALL:
		Color = FColor::Black;
		break;
	case LEVEL_EDGE:
		Color = FColor::Purple;
		break;
	case STREET_HOR:
		Color = COL_STREET_HOR;
		break;
	case STREET_VERT:
		Color = COL_STREET_VERT;
		break;
	case INTERSECTION:
		Color = FColor::White;
		break;
	default:
		break;
	}
}

void Tile::SetRegion(const TileRegion region)
{
	m_Region = region;
	switch (region)
	{
	case EDGE:
		break;
	case CENTER:
		break;
	case ENTRANCE:
		break;
	default:
		break;
	}
}
