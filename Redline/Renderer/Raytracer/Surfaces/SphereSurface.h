#pragma once
#include <mathfu/glsl_mappings.h>
#include "Surface.h"
#include "../../../GraphicalResources/Material.h"

namespace Redline
{
	//Represents a sphere surface in a scene
	class SphereSurface : public Surface
	{
	public:
		SphereSurface(const mathfu::vec3& position, float radius, std::shared_ptr<Material>& material);
		SphereSurface(const mathfu::vec3& position, float radius);

		bool AttemptRayIntersection(const Ray& ray, float& outIntersectionDistance, RayHitSurfaceDetails& outSurfaceDetails) override;

		bool CheckRayOcclusion(const Ray& ray, float maxDistance) override;

		BoundingBox GetBounds() override;

	private:
		mathfu::vec3 _position;
		float _radius;
		float _radiusSquared;
		std::shared_ptr<Material> _surfaceMaterial;
	};
}
