#pragma once
#include <mathfu/glsl_mappings.h>
#include "Surface.h"
#include "../../../GraphicalResources/Material.h"
#include "../../../Scene/Transform.h"
#include "../../../GraphicalResources/TriangleMesh.h"
#include "SphereSurface.h"
#include "../../../Math/BoundingBox.h"
#include "Math/BVH/BVH.h"

namespace Redline
{
	//Represents a sphere surface in a scene
	class TriangleMeshSurface : public Surface
	{
	public:
		TriangleMeshSurface(const Transform& objectTransform, std::shared_ptr<TriangleMesh>& mesh, std::vector<std::shared_ptr<Material>>& materials);

		bool AttemptRayIntersection(const Ray& ray, float& outIntersectionDistance, RayHitSurfaceDetails& outSurfaceDetails) override;

		bool RayTriangleIntersection(const Ray& localRay
			, const mathfu::vec3& vertA
			, const mathfu::vec3& vertB
			, const mathfu::vec3& vertC,
			float& outHitDistance);

		bool CheckRayOcclusion(const Ray& ray, float maxDistance) override;

	private:

		mathfu::vec3 ComputeInterpolatedPoints(
			const mathfu::vec3& vertA,
			const mathfu::vec3& vertB,
			const mathfu::vec3& vertC,
			const mathfu::vec3& hitPoint
		);

		BoundingBox GetBounds() override;


		bool AttemptRayIntersection_TriangleBucket(const Ray& ray, const std::vector<MeshTriangle>& triangles, float& outIntersectionDistance, RayHitSurfaceDetails& outSurfaceDetails);
		bool CheckOcclusion_TriangleBucket(const Ray& ray, const std::vector<MeshTriangle>& triangles, float maxDistance);

		bool AttemptRayIntersection_BruteForce(const Ray& ray, float& outIntersectionDistance, RayHitSurfaceDetails& outSurfaceDetails);
		bool CheckOcclusion_BruteForce(const Ray& ray, float maxDistance);

		bool AttemptRayIntersection_AcclerationTreeNode(const Ray& ray, KDTreeNode* currentNode,float entry, float exit, float& outIntersectionDistance, RayHitSurfaceDetails& outSurfaceDetails);
		bool CheckOcclusion_AcclerationTreeNode(const Ray& ray, KDTreeNode* currentNode, float entry, float exit, float maxDistance);

		bool AttemptRayIntersection_BVH(const Ray& ray, BVHNode<BVHTriangle>* currentNode, float entry, float exit, float& outIntersectionDistance, RayHitSurfaceDetails& outSurfaceDetails);
		bool CheckOcclusion_BVH(const Ray& ray, BVHNode<BVHTriangle>* currentNode, float entry, float exit, float maxDistance);


		RayHitSurfaceDetails GetSurfaceForHit(const Ray& ray, float distance, const std::vector<MeshTriangle>& triangles, unsigned triangleIndex);

		RayHitSurfaceDetails GetSurfaceForHit(const Ray& ray, float distance, BVHTriangle& triangle);

		mathfu::mat4 _meshWorldMatrix;
		mathfu::mat4 _inverseMeshWorldMatrix;

		std::shared_ptr<TriangleMesh> _mesh;

		KDTreeNode* _acclerationTree;
		BVHNode<BVHTriangle>* _bvh;

		std::vector<std::shared_ptr<Material>> _surfaceMaterials;

		SphereSurface _boundingSurface;
		BoundingBox _boundingBoxWorldspace;
		BoundingBox _boundingBoxObjectSpace;
	};
}
