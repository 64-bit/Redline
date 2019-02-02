#pragma once
#include "Math/ImageRectangle.h"


namespace Redline
{
	class RenderTileInfo
	{
	public:
		ImageRectangle ImageRegion;

		unsigned CurrentSamples;

		static bool CompareTiles(RenderTileInfo* left, RenderTileInfo* right);
	};
}
