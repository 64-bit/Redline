#pragma once
#include "mathfu/glsl_mappings.h"
#include "../GraphicalResources/MeshVertex.h"
#include <vector>
#include "../GraphicalResources/MeshTriangle.h"

namespace Redline
{
	struct Ray;

	class BoundingBox
	{
	public:
		mathfu::vec3 Min;
		mathfu::vec3 Max;

		BoundingBox();

		bool DoesRayIntersect(const Ray& ray, float& outTNear, float& outTFar);

		void EnlargeByPoints(const std::vector<MeshVertex>& verticies);
		void EnlargeByPoint(const mathfu::vec3& point);

		void EnlargeByTriangles(const std::vector<MeshTriangle>& triangles, const std::vector<MeshVertex>& verticies);

		void EnlargeByBounds(const BoundingBox& otherBounds);

		mathfu::vec3 GetCenter() const;

	private:
	};
}
