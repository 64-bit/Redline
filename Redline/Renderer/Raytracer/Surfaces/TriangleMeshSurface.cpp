#include "stdafx.h"
#include "TriangleMeshSurface.h"
#include "../../../Math/Ray.h"
#include <limits>
#include <CompileTimeSettings.h>

using namespace Redline;
using namespace mathfu;
using namespace std;

TriangleMeshSurface::TriangleMeshSurface(const Transform & objectTransform, std::shared_ptr<TriangleMesh>& mesh, std::vector<std::shared_ptr<Material>>& materials)
	:_boundingSurface(objectTransform.GetPosition(), mesh->BoundingRadius * objectTransform.GetLocalScale().x)
{
	_meshWorldMatrix = objectTransform.GetWorldTransformMatrix();
	_inverseMeshWorldMatrix = _meshWorldMatrix.Inverse();
	_mesh = mesh; //TODO: Copy mesh
	//Copy material so that the scene can be changed while we render
	_surfaceMaterials = materials;

	_boundingBoxObjectSpace.EnlargeByPoints(_mesh->Verticies);

	//Transform bounds into world space
	for(auto& vertex : _mesh->Verticies)
	{
		vec4 wsVertex = _meshWorldMatrix * vec4(vertex.Position, 1.0f);
		_boundingBoxWorldspace.EnlargeByPoint(wsVertex.xyz());
	}

	std::vector<MeshTriangle> triangles = mesh->Triangles;

	if (_mesh->GetTriangleCount() > 32)
	{
		if(CompileTimeSettings::USE_BVH)
		{
			vector<BVHTriangle> bvhTris;
			for(auto& triangle : triangles)
			{
				//Construct bvh tris from this
				BVHTriangle tris;

				tris.Material = triangle.Material;
				tris.A = _mesh->Verticies[triangle.A];
				tris.B = _mesh->Verticies[triangle.B];
				tris.C = _mesh->Verticies[triangle.C];
				bvhTris.push_back(tris);
			}
			_bvh = BVHNode<BVHTriangle>::BuildBVH(bvhTris, 16);

		}
		else
		{
			_acclerationTree = new KDTreeNode();
			_acclerationTree->InsertPoints(triangles, _mesh->Verticies, 2);
		}
	}
	else
	{
		_acclerationTree = nullptr;
	}
}

const float EPSILON = 0.0000001f;

//Compute ray triangle intersection using Möller–Trumbore
//https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
bool TriangleMeshSurface::AttemptRayIntersection(const Ray& ray, float& outIntersectionDistance,
	RayHitSurfaceDetails& outSurfaceDetails)
{
	float entry, exist;
	if (!_boundingBoxWorldspace.DoesRayIntersect(ray, entry, exist))
	{
		return false; //If we don't hit our bounding sphere, just return failure
	}

	if(entry >= outIntersectionDistance)
	{
		return false;//Early out on already occluded
	}

	//Transform ray into local space
	Ray localRay;
	localRay.Origin = (_inverseMeshWorldMatrix * vec4(ray.Origin, 1.0f)).xyz();
	localRay.Direction = (_inverseMeshWorldMatrix * vec4(ray.Direction, 0.0f)).xyz();
	//localRay.Direction.Normalize();

	RayHitSurfaceDetails localHitDetails;
	bool didHit = false;

	if (_acclerationTree != nullptr
		|| _bvh != nullptr)
	{
		//return AttemptRayIntersection_BruteForce(localRay, outIntersectionDistance, outSurfaceDetails);

		Ray fixedLocalRay = localRay;
		for (int i = 0; i < 3; i++)
		{
			if (fixedLocalRay.Direction[i] == 0.0f)
			{
				fixedLocalRay.Direction[i] = 0.0000001f;
			}
		}
		if (CompileTimeSettings::USE_ACCL)
		{
			if(CompileTimeSettings::USE_BVH)
			{
				float localIntersectionDistance = std::numeric_limits<float>::max();
				didHit = AttemptRayIntersection_BVH(fixedLocalRay, _bvh, entry, exist, localIntersectionDistance, localHitDetails);
				if(didHit)
				{
					outIntersectionDistance = localIntersectionDistance;
				}
			}
			else
			{
				didHit = AttemptRayIntersection_AcclerationTreeNode(fixedLocalRay, _acclerationTree, entry, exist, outIntersectionDistance, localHitDetails);
			}
		}
		else
		{
			didHit =  AttemptRayIntersection_BruteForce(localRay, outIntersectionDistance, localHitDetails);
		}
	}
	else
	{
		didHit =  AttemptRayIntersection_BruteForce(localRay, outIntersectionDistance, localHitDetails);
	}

	if(!didHit)
	{
		return false;
	}

	//Fix position and normal on interseciton

	localHitDetails.SurfacePosition = (_meshWorldMatrix * vec4(localHitDetails.SurfacePosition, 1.0f)).xyz();
	localHitDetails.SurfaceNormal = (_meshWorldMatrix * vec4(localHitDetails.SurfaceNormal, 0.0f)).xyz().Normalized();
	localHitDetails.SurfaceNormal_Geometry = (_meshWorldMatrix * vec4(localHitDetails.SurfaceNormal_Geometry, 0.0f)).xyz().Normalized();

	outSurfaceDetails = localHitDetails;

	//this fixes the hit distance if the mesh has scaling
	outIntersectionDistance = vec3::Distance(ray.Origin, localHitDetails.SurfacePosition);

	return true;
}

bool TriangleMeshSurface::CheckRayOcclusion(const Ray& ray, float maxDistance)
{
	//Transform ray into local space
	Ray localRay;
	localRay.Origin = (_inverseMeshWorldMatrix * vec4(ray.Origin, 1.0f)).xyz();
	localRay.Direction = (_inverseMeshWorldMatrix * vec4(ray.Direction, 0.0f)).xyz();
	//localRay.Direction.Normalize();

	float entry, exist;
	if (!_boundingBoxObjectSpace.DoesRayIntersect(localRay, entry, exist))
	{
		return false; //If we don't hit our bounding sphere, just return failure
	}

	if (_acclerationTree != nullptr)
	{
		//return AttemptRayIntersection_BruteForce(localRay, outIntersectionDistance, outSurfaceDetails);

		Ray fixedLocalRay = localRay;
		for (int i = 0; i < 3; i++)
		{
			if (fixedLocalRay.Direction[i] == 0.0f)
			{
				fixedLocalRay.Direction[i] = 0.0000001f;
			}
		}
		if (CompileTimeSettings::USE_ACCL)
		{
			if(CompileTimeSettings::USE_BVH)
			{
				return CheckOcclusion_BVH(fixedLocalRay, _bvh, entry, exist, maxDistance);
			}
			else
			{
				return CheckOcclusion_AcclerationTreeNode(fixedLocalRay, _acclerationTree, entry, exist, maxDistance);
			}
		}
		else
		{
			return CheckOcclusion_BruteForce(localRay, maxDistance);
		}
	}
	else
	{
		return CheckOcclusion_BruteForce(localRay, maxDistance);
	}
}

vec3 TriangleMeshSurface::ComputeInterpolatedPoints(
	const vec3& vertA,
	const vec3& vertB,
	const vec3& vertC,
	const vec3& hitPoint)
{
	vec3 v0 = vertB - vertA, v1 = vertC - vertA, v2 = hitPoint - vertA;
	float d00 = vec3::DotProduct(v0, v0);
	float d01 = vec3::DotProduct(v0, v1);
	float d11 = vec3::DotProduct(v1, v1);
	float d20 = vec3::DotProduct(v2, v0);
	float d21 = vec3::DotProduct(v2, v1);
	float denom = d00 * d11 - d01 * d01;

	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0f - v - w;

	return vec3(u, v, w);
}

BoundingBox TriangleMeshSurface::GetBounds()
{
	return _boundingBoxWorldspace;
}

bool TriangleMeshSurface::AttemptRayIntersection_TriangleBucket(const Ray& ray, const std::vector<MeshTriangle>& triangles, float & outIntersectionDistance, RayHitSurfaceDetails& outSurfaceDetails)
{
	float closestHit = FLT_MAX;
	bool hasHit = false;
	unsigned closestHitIndex = 0;

	const unsigned triangleCount = (unsigned)triangles.size();
	for (unsigned i = 0; i < triangleCount; i++)
	{
		//Load mesh vertices
		vec3 vertA = _mesh->VerticiePositionsOnly[triangles[i].A];
		vec3 vertB = _mesh->VerticiePositionsOnly[triangles[i].B];
		vec3 vertC = _mesh->VerticiePositionsOnly[triangles[i].C];

		//Preform ray intersection
		float thisHitDistance;

		bool didHit = RayTriangleIntersection(ray, vertA, vertB, vertC, thisHitDistance);

		if (!didHit)
		{
			continue;
		}

		if (thisHitDistance >= closestHit)
		{
			continue;
		}

		//If we hit, and are the closest process the material and construct the ray hit surface details
		hasHit = true;
		closestHit = thisHitDistance;
		closestHitIndex = i;
	}

	if(hasHit)
	{
		outSurfaceDetails = GetSurfaceForHit(ray, closestHit,triangles, closestHitIndex);
	}

	outIntersectionDistance = closestHit;
	return hasHit;
}

bool TriangleMeshSurface::CheckOcclusion_TriangleBucket(const Ray & ray, const std::vector<MeshTriangle>& triangles,float maxDistance)
{
	const unsigned triangleCount = (unsigned)triangles.size();
	for (unsigned i = 0; i < triangleCount; i++)
	{
		//Load mesh vertices
		vec3 vertA = _mesh->VerticiePositionsOnly[triangles[i].A];
		vec3 vertB = _mesh->VerticiePositionsOnly[triangles[i].B];
		vec3 vertC = _mesh->VerticiePositionsOnly[triangles[i].C];

		//Preform ray intersection
		float hitDistanceResult;

		bool didHit = RayTriangleIntersection(ray, vertA, vertB, vertC, hitDistanceResult);

		if (didHit && hitDistanceResult <= maxDistance)
		{
			return true;
		}
	}

	return false;
}

RayHitSurfaceDetails TriangleMeshSurface::GetSurfaceForHit(const Ray & ray, float distance, const std::vector<MeshTriangle>& triangles,  unsigned triangleIndex)
{
	auto hitPoint = ray.Origin + ray.Direction * distance;
	auto& triangle = triangles[triangleIndex];

	const auto& vertexA = _mesh->Verticies[triangle.A];
	const auto& vertexB = _mesh->Verticies[triangle.B];
	const auto& vertexC = _mesh->Verticies[triangle.C];

	auto interpolatedPoints = ComputeInterpolatedPoints(
		vertexA.Position,
		vertexB.Position,
		vertexC.Position,
		hitPoint);

	auto normal = 
		vertexA.Normal * interpolatedPoints.x +
		vertexB.Normal * interpolatedPoints.y +
		vertexC.Normal * interpolatedPoints.z;

	auto uv =
		vertexA.UV * interpolatedPoints.x +
		vertexB.UV * interpolatedPoints.y +
		vertexC.UV * interpolatedPoints.z;

	auto tangent =
		vertexA.Tangent * interpolatedPoints.x +
		vertexB.Tangent * interpolatedPoints.y +
		vertexC.Tangent * interpolatedPoints.z;

	auto biNormal =
		vertexA.BiNormal * interpolatedPoints.x +
		vertexB.BiNormal * interpolatedPoints.y +
		vertexC.BiNormal * interpolatedPoints.z;

	//uv.y = 1.0f - uv.y;

	normal.Normalize();

	//For now, we will just compute the tangent at runtime. in the future, we should cache this
	RayHitSurfaceDetails surface;

	bool flipNormal = vec3::DotProduct(normal, ray.Direction) > 0.0f;
	surface.SurfaceNormal = flipNormal ? -normal : normal;
	surface.SurfaceNormal_Geometry = surface.SurfaceNormal;

	surface.TangentMatrix.GetColumn(0) = tangent.Normalized();
	surface.TangentMatrix.GetColumn(1) = biNormal.Normalized();
	surface.TangentMatrix.GetColumn(2) = surface.SurfaceNormal.Normalized();


	surface.SurfacePosition = ray.Origin + ray.Direction * distance;

	_surfaceMaterials[triangle.Material]->ApplyToSurfaceHit(surface, uv, 0.0f);

	return surface;
}

RayHitSurfaceDetails TriangleMeshSurface::GetSurfaceForHit(const Ray& ray, float distance, BVHTriangle& triangle)
{
	auto hitPoint = ray.Origin + ray.Direction * distance;

	const auto& vertexA = triangle.A;
	const auto& vertexB = triangle.B;
	const auto& vertexC = triangle.C;

	auto interpolatedPoints = ComputeInterpolatedPoints(
		vertexA.Position,
		vertexB.Position,
		vertexC.Position,
		hitPoint);

	auto normal =
		vertexA.Normal * interpolatedPoints.x +
		vertexB.Normal * interpolatedPoints.y +
		vertexC.Normal * interpolatedPoints.z;

	auto uv =
		vertexA.UV * interpolatedPoints.x +
		vertexB.UV * interpolatedPoints.y +
		vertexC.UV * interpolatedPoints.z;

	auto tangent =
		vertexA.Tangent * interpolatedPoints.x +
		vertexB.Tangent * interpolatedPoints.y +
		vertexC.Tangent * interpolatedPoints.z;

	auto biNormal =
		vertexA.BiNormal * interpolatedPoints.x +
		vertexB.BiNormal * interpolatedPoints.y +
		vertexC.BiNormal * interpolatedPoints.z;

	//uv.y = 1.0f - uv.y;

	normal.Normalize();

	//For now, we will just compute the tangent at runtime. in the future, we should cache this
	RayHitSurfaceDetails surface;

	bool flipNormal = vec3::DotProduct(normal, ray.Direction) > 0.0f;
	surface.SurfaceNormal = flipNormal ? -normal : normal;
	surface.SurfaceNormal_Geometry = surface.SurfaceNormal;

	surface.TangentMatrix.GetColumn(0) = tangent.Normalized();
	surface.TangentMatrix.GetColumn(1) = biNormal.Normalized();
	surface.TangentMatrix.GetColumn(2) = surface.SurfaceNormal.Normalized();


	surface.SurfacePosition = ray.Origin + ray.Direction * distance;

	_surfaceMaterials[triangle.Material]->ApplyToSurfaceHit(surface, uv, 0.0f);

	return surface;
}

bool TriangleMeshSurface::AttemptRayIntersection_BruteForce(const Ray& ray, float& outIntersectionDistance, RayHitSurfaceDetails & outSurfaceDetails)
{
	return AttemptRayIntersection_TriangleBucket(ray, _mesh->Triangles, outIntersectionDistance, outSurfaceDetails);
}

bool Redline::TriangleMeshSurface::CheckOcclusion_BruteForce(const Ray & ray, float maxDistance)
{
	return CheckOcclusion_TriangleBucket(ray, _mesh->Triangles, maxDistance);
}

bool TriangleMeshSurface::AttemptRayIntersection_AcclerationTreeNode(const Ray& ray, KDTreeNode* currentNode, float entry, float exit, float& outIntersectionDistance, RayHitSurfaceDetails& outSurfaceDetails)
{
	int splitPlane = currentNode->SplitPlane;

	float rayDir = ray.Direction[splitPlane];
	float rayPos = ray.Origin[splitPlane];

	float toPlane = currentNode->SplitPoint - rayPos;
	float t = toPlane / rayDir;

	KDTreeNode* near;
	KDTreeNode* far;

	if (rayDir > 0.0f)
	{
		near = currentNode->Left;
		far = currentNode->Right;
	}
	else
	{
		near = currentNode->Right;
		far = currentNode->Left;
	}

	float closestIntersection = FLT_MAX;
	RayHitSurfaceDetails closestSurfaceDetails;
	bool hasClosestHit = false;

	//Do all local triangles, but only if we intersect this bucket
	float junkNear, junkFar;
	bool doesHitLocalTriangles = currentNode->LocalTriangleBounds.DoesRayIntersect(ray, junkNear, junkFar);

//	doesHitLocalTriangles = true;

	if(doesHitLocalTriangles)
	{
		hasClosestHit = AttemptRayIntersection_TriangleBucket(ray, currentNode->Triangles, closestIntersection, closestSurfaceDetails);
	}

	if (t >= exit && near != nullptr)
	{
		//Near only, split is after exsit
		float localIntersection = FLT_MAX;
		RayHitSurfaceDetails localSurface;

		if (AttemptRayIntersection_AcclerationTreeNode(ray, near, entry,exit, localIntersection, localSurface))
		{
			if (localIntersection < closestIntersection)
			{
				closestIntersection = localIntersection;
				closestSurfaceDetails = localSurface;
				hasClosestHit = true;
			}
		}

	}
	else if (t <= entry && far != nullptr)
	{
		//far only we entered the volume before the split
		//Near only, split is after exsit
		float localIntersection = FLT_MAX;
		RayHitSurfaceDetails localSurface;

		if (AttemptRayIntersection_AcclerationTreeNode(ray, far, entry, exit, localIntersection, localSurface))
		{
			if (localIntersection < closestIntersection)
			{
				closestIntersection = localIntersection;
				closestSurfaceDetails = localSurface;
				hasClosestHit = true;
			}
		}
	}
	else
	{
		//Do near then far if no intersections are found in either


		float localIntersection = FLT_MAX;
		RayHitSurfaceDetails localSurface;
		//Near

		if (near != nullptr &&
			AttemptRayIntersection_AcclerationTreeNode(ray, near, entry, exit, localIntersection, localSurface))
		{
			if (localIntersection < closestIntersection)
			{
				closestIntersection = localIntersection;
				closestSurfaceDetails = localSurface;
				hasClosestHit = true;
			}
			//Early out before far
		}

		//Far
		if (far != nullptr &&									//We pass t as entry to far, but only in this case.
			AttemptRayIntersection_AcclerationTreeNode(ray, far, t, exit, localIntersection, localSurface))
		{
			if (localIntersection < closestIntersection)
			{
				closestIntersection = localIntersection;
				closestSurfaceDetails = localSurface;
				hasClosestHit = true;
			}
			//Early out before far
		}
	}

	if (hasClosestHit)
	{
		outIntersectionDistance = closestIntersection;
		outSurfaceDetails = closestSurfaceDetails;
	}
	return hasClosestHit;
}

bool TriangleMeshSurface::CheckOcclusion_AcclerationTreeNode(const Ray& ray, KDTreeNode* currentNode, float entry, float exit, float maxDistance)
{

	if(entry > maxDistance)//If we enter this node past our max distance, there is no need to check
	{
		return false;
	}

	int splitPlane = currentNode->SplitPlane;

	float rayDir = ray.Direction[splitPlane];
	float rayPos = ray.Origin[splitPlane];

	float toPlane = currentNode->SplitPoint - rayPos;
	float t = toPlane / rayDir;

	KDTreeNode* near;
	KDTreeNode* far;

	if (rayDir > 0.0f)
	{
		near = currentNode->Left;
		far = currentNode->Right;
	}
	else
	{
		near = currentNode->Right;
		far = currentNode->Left;
	}

	//Do all local triangles, but only if we intersect this bucket
	float junkNear, junkFar;
	bool doesHitLocalTriangles = currentNode->LocalTriangleBounds.DoesRayIntersect(ray, junkNear, junkFar);

	//	doesHitLocalTriangles = true;

	if (doesHitLocalTriangles)
	{
		if(CheckOcclusion_TriangleBucket(ray, currentNode->Triangles, maxDistance))
		{
			return true;
		}
	}

	if (t >= exit && near != nullptr)
	{
		//Near only, split is after exsit
		float localIntersection = FLT_MAX;
		RayHitSurfaceDetails localSurface;

		if (CheckOcclusion_AcclerationTreeNode(ray, near, entry, exit, maxDistance))
		{
			return true;
		}

	}
	else if (t <= entry && far != nullptr)
	{
		//far only we entered the volume before the split
		//Near only, split is after exsit
		float localIntersection = FLT_MAX;
		RayHitSurfaceDetails localSurface;

		if (CheckOcclusion_AcclerationTreeNode(ray, far, entry, exit, maxDistance))
		{
			return true;
		}
	}
	else
	{
		//Do near then far if no intersections are found in either

		//Near
		if (near != nullptr &&
			CheckOcclusion_AcclerationTreeNode(ray, near, entry, exit, maxDistance))
		{
			return true;
		}

		//Far
		if (far != nullptr &&									//We pass t as entry to far, but only in this case.
			CheckOcclusion_AcclerationTreeNode(ray, far, t, exit, maxDistance))
		{
			return true;
		}
	}

	return false;
}

bool TriangleMeshSurface::AttemptRayIntersection_BVH(const Ray& ray, BVHNode<BVHTriangle>* bvhNode, float entry, float exit,
	float& outIntersectionDistance, RayHitSurfaceDetails& outSurfaceDetails)
{
	float oNear, oFar;
	//Try node bounds,
	if(!bvhNode->Bounds.DoesRayIntersect(ray,oNear,oFar))
	{
		return false;
	}

	bool anyValidHits = false;

	//If has children, try all children
	if(bvhNode->ChildNodes.size() > 0)
	{
		for(auto& child : bvhNode->ChildNodes)
		{
			float thisIntersectionDistance = std::numeric_limits<float>::max();
			RayHitSurfaceDetails thisHitSurface;
			if(AttemptRayIntersection_BVH(ray,child,entry,exit, thisIntersectionDistance, thisHitSurface))
			{
				anyValidHits = true;
				if(thisIntersectionDistance < outIntersectionDistance)
				{
					outIntersectionDistance = thisIntersectionDistance;
					outSurfaceDetails = thisHitSurface;
				}
			}
		}
	}
	else//Else, try all contents
	{
		for(auto& triangle : bvhNode->ContentsVector)
		{
			float dist;
			if (RayTriangleIntersection(ray,
				triangle.A.Position,
				triangle.B.Position,
				triangle.C.Position,
				dist))
			{
				anyValidHits = true;
				if(dist < outIntersectionDistance)
				{
					outIntersectionDistance = dist;
					outSurfaceDetails = GetSurfaceForHit(ray, dist, triangle);
				}
			}
		}
	}
	
	return anyValidHits;
}

bool TriangleMeshSurface::CheckOcclusion_BVH(const Ray& ray, BVHNode<BVHTriangle>* bvhNode, float entry, float exit,
	float maxDistance)
{
	float oNear, oFar;
	//Try node bounds,
	if (!bvhNode->Bounds.DoesRayIntersect(ray, oNear, oFar))
	{
		return false;
	}

	if(oNear > maxDistance)
	{
		return false;
	}

	//If has children, try all children
	if (bvhNode->ChildNodes.size() > 0)
	{
		for (auto& child : bvhNode->ChildNodes)
		{
			if (CheckOcclusion_BVH(ray, child, entry, exit, maxDistance))
			{
				return true;
			}
		}
	}
	else//Else, try all contents
	{
		for (auto& triangle : bvhNode->ContentsVector)
		{
			float dist;
			if (RayTriangleIntersection(ray,
				triangle.A.Position,
				triangle.B.Position,
				triangle.C.Position,
				dist))
			{
				if (dist < maxDistance)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool TriangleMeshSurface::RayTriangleIntersection(const Ray& localRay, const vec3& vertA, const vec3& vertB,
	const vec3& vertC, float& outHitDistance)
{
	vec3 edge1 = vertB - vertA;
	vec3 edge2 = vertC - vertA;

	vec3 h = vec3::CrossProduct(localRay.Direction, edge2);
	float a = vec3::DotProduct(edge1, h);

	
	if (a > -EPSILON && a < EPSILON)
	{
		return false;
	}

	/*if (a < EPSILON) Replace the above with this for backface culling
	{
		return false;
	}*/

	float f = 1.0f / a;
	vec3 s = localRay.Origin - vertA;
	float u = f * vec3::DotProduct(s, h);
	if (u < 0.0 || u > 1.0)
	{
		return false;
	}

	vec3 q = vec3::CrossProduct(s, edge1);
	float v = f * vec3::DotProduct(localRay.Direction, q);
	if (v < 0.0 || u + v > 1.0)
	{
		return false;
	}

	float t = f * vec3::DotProduct(edge2, q);

	if (t > EPSILON) // ray intersection
	{
		outHitDistance = t;
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
	{
		return false;
	}
}
