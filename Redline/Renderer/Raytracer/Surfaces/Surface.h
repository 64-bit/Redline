#pragma once
#include "../RayHitSurfaceDetails.h"
#include "../../../Math/BoundingBox.h"

namespace Redline
{
	struct Ray;

	//Base class for all types of visable surfaces.
	//The surface class is responsible for computing intersections with rays against itself,
	//and in the case of a hit, returning the surface properties of the hit
	class Surface
	{
	public:
		//Test ray

		virtual bool AttemptRayIntersection(const Ray& ray, float& outIntersectionDistance, RayHitSurfaceDetails& outSurfaceDetails) = 0;

		//This should test if the ray hits any part of the surface, we don't care what it is
		//This is used primarily for shadows
		virtual bool CheckRayOcclusion(const Ray& ray, float maxDistance = FLT_MAX) = 0;

		//Return a copy as a sphere surface does not have bounds
		virtual BoundingBox GetBounds() = 0;
	};
}
