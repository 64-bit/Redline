#pragma once
#include <mathfu/glsl_mappings.h>
#include "../../Raytracer.h"

namespace Redline
{
	class Light
	{
	public:
		//Returns the radiance from the light at a given point, without shadows
		virtual mathfu::vec3 GetRadianceAtPoint(const mathfu::vec3& point) const = 0;

		//Returns the radiance in the out value from the light at a given point, taking shadows into effcet
		//returns true if any light is receved, or flase if 100% shadowed
		//0 Samples = Shadows disabled
		//1 Sample = Sample just the center
		//2+ Samples = Sample distrubution of physical light surface
		virtual bool GetRadianceAtPointShadowed(const mathfu::vec3& point, Raytracer& raytracer, bool enableSoftShadows, int maxShadowSamples, mathfu::vec3& outRadiance) const = 0;

		//Can the light affect a given bounding box in any visable way. used for culling the list of lights
		//that affect a surface
		virtual bool CanLightAffectBounds(const BoundingBox& bounds) const = 0;
	};
}
