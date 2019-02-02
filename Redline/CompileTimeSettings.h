#pragma once
//Contains compile time settings for the project

namespace Redline
{
	namespace CompileTimeSettings
	{
		//Use acceleration structure for triangle meshes
		const bool USE_ACCL = true;
		//Use bounding volume hierarchy for triangle meshes(should always be true for now)
		const bool USE_BVH = true;
		//Index of refraction for air
		const float IOR_AIR = 1.0f;
		//Surface offset to prevent self intersection with new ray
		const float NEXT_RAY_SURFACE_OFFSET = 0.00001f;

		//Frame rendering

		//Size of render tiles in pixels
		const int BLOCK_SIZE = 16;
		//Clamp value for samples (less accurate results if enabled) a value of 0 or less will disable
		const float SAMPLE_CLAMP = 10.0f;
		//Use progressive rendering mode
		const bool USE_PROGRESSIVE_MODE = true;
		//Number of samples per pass in progressive mode
		const unsigned PROGRESSIVE_IMPROVE_SAMPELS = 16;
	}
}