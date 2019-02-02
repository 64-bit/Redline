#pragma once
#include "../Raytracer.h"
#include "../ViewRay.h"
#include "../SceneLightingData.h"

namespace Redline
{
	//Abstract base class for all BRDF implementations
	//In the render, a BRDF is responsible for shading the results of inital ray hits.
	//A BRDF is allowed to trace and require the shading of many additional rays for things like reflections
	//The base class has a number of utilities for common cases like basic reflections

	//The Raytracer will call ShadeRayStart for each raystart (sample) that the render settings require.
	//The BRDF is responsible for ALL raytracing, including the original ray

	//This class is provided with the raytracer, a light list and the frame quailty settings.

	class BRDF
	{
	public:

		virtual mathfu::vec3 ImportanceSampleSpecular(const mathfu::vec3& surfaceNormal, const mathfu::vec2& uniformRandom, float roughness) = 0;

		virtual mathfu::vec3 ComputeReflectanceSpecular(float& outProbability, const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay,
			const mathfu::vec3& surfaceToLight) = 0;

		virtual mathfu::vec3 ComputeReflectanceDiffuse(float& outProbability, const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay,
			const mathfu::vec3& surfaceToLight) = 0;

	protected:
		BRDF();
	};
}
