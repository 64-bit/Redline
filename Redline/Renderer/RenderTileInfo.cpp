#include "stdafx.h"
#include "RenderTileInfo.h"

bool Redline::RenderTileInfo::CompareTiles(RenderTileInfo * left, RenderTileInfo * right)
{
	return left->CurrentSamples > right->CurrentSamples;
}
