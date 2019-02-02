#pragma once
#include "mathfu/glsl_mappings.h"
#include "Light.h"

namespace Redline
{
	class PointLight : public Light
	{
	public:
		mathfu::vec3  Color;
		float Power;
		float LightRadius;
		float PhysicalRadius;
		mathfu::vec3 Position;

		float GetFalloff(float distance) const;

		mathfu::vec3 GetRadianceAtPoint(const mathfu::vec3& point) const override;

		//Used for sampling the light in a more accurate manner, as the radiance will vary based
		//on the distance of the surface we are shading, to the physical surface of the light, and not
		//the center of the light
		mathfu::vec3 GetRadianceAtDistance(float distance) const;

		bool GetRadianceAtPointShadowed(const mathfu::vec3& point, Raytracer& raytracer, bool enableSoftShadows, int maxShadowSamples, mathfu::vec3& outRadiance) const override;
		bool CanLightAffectBounds(const BoundingBox& bounds) const override;
	};
}
