#pragma once
#include "mathfu/glsl_mappings.h"
#include "Light.h"

namespace Redline
{
	class DirectionalLight : public Light
	{
	public:
		mathfu::vec3  Color;
		float Power;
		mathfu::vec3 Direction;

		float PhysicalDistance;
		float PhysicalRadius;

		mathfu::vec3 GetRadianceAtPoint(const mathfu::vec3& point)const override;
		bool GetRadianceAtPointShadowed(const mathfu::vec3& point, Raytracer& raytracer, bool enableSoftShadows, int maxShadowSamples, mathfu::vec3& outRadiance)const override;
		bool CanLightAffectBounds(const BoundingBox& bounds)const override;
	};
}
