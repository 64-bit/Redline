#pragma once

#include "../Math/Ray.h"

namespace Redline
{
	//A view ray is a inital ray fired from a camera into the scene.
	//It contains additional metadata about the ray
	struct ViewRay : Ray
	{
		mathfu::vec2 ScreenSpaceOrigin; //-1.0f to 1.0f screen space position of the ray 
		unsigned PixelX; //X Pixel this ray is for
		unsigned PixelY; //Y Pixel this ray is for
		unsigned SampleID; //Which sample for the given pixel is this ray.
	};
}