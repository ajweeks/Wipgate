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

void Tile::SetType(const TileType type)
{
	m_Type = type;
	switch (type)
	{
	case FLOOR:
		Color = FColor::White;
		break;
	case WALL:
		Color = FColor::Black;
		break;
	case STREET:
		Color = FColor::Yellow;
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
