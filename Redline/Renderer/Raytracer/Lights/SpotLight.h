#pragma once
#include "mathfu/glsl_mappings.h"
#include "Light.h"

namespace Redline
{
	class SpotLight : public Light
	{
	public:
		mathfu::vec3  Color;
		float Power;
		float LightRadius;
		float PhysicalRadius;
		mathfu::vec3 Position;
		mathfu::vec3 Direction;

		float InnerAngleCos;
		float OuterAngleCos;

		float GetFalloff(float distance, float cosTheta) const;

		mathfu::vec3 GetRadianceAtPoint(const mathfu::vec3& point) const override;

		//Used for sampling the light in a more accurate manner, as the radiance will vary based
		//on the distance of the surface we are shading, to the physical surface of the light, and not
		//the center of the light
		mathfu::vec3 GetRadianceAtDistance(float distance, float cosTheta) const;

		bool GetRadianceAtPointShadowed(const mathfu::vec3& point, Raytracer& raytracer, bool enableSoftShadows, int maxShadowSamples, mathfu::vec3& outRadiance) const override;
		bool CanLightAffectBounds(const BoundingBox& bounds) const override;
	};
}
