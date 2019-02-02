#include "stdafx.h"
#include "Raytracer.h"
#include "../Scene/SceneObject.h"
#include "../GraphicalResources/Color.h"
#include "../Scene/Components/Renderable/RenderableSceneComponent.h"
#include  <math.h>
#include "../Math/Math.h"
#include <limits>
#include "../Scene/Components/Lights/DirectionalLightComponent.h"
#include "../Math/BVH/BVH.h"
#include "../Math/Ray.h"
#include <stack>

using namespace Redline;
using namespace mathfu;
using namespace std;

bool Raytracer::BuildSceneStructure(const std::shared_ptr<Scene>& scene)
{
	_surfaces.clear();

	//Bind GetSurfacesFromSceneObject to this surface list, and then invoke Scene::ForEachSceneObject
	std::function<bool(SceneObject*)> boundCallback =
		std::bind(&Raytracer::GetSurfacesFromSceneObject,std::placeholders::_1, &_surfaces);

	scene->ForEachSceneObject(boundCallback);

	//Build BVH
	_sceneBVH = BVHNode<Surface*>::BuildBVH(_surfaces, 4);

	_isReady = true;

	return true;
}

bool Raytracer::GetIsReady() const
{
	return _isReady;
}

bool Raytracer::RaytraceOcclusion(const Ray & ray, float maxDistance)
{
	//Raytrace BVH
	stack<BVHNode<Surface*>*> nodesToVisit;

	float n, f;
	if (_sceneBVH->Bounds.DoesRayIntersect(ray, n, f))
	{
		nodesToVisit.push(_sceneBVH);
	}

	while (nodesToVisit.size() > 0)
	{
		auto node = nodesToVisit.top();;
		nodesToVisit.pop();

		//Any node added to this has already passed the hit test, so we check all the children

		//Check all children nodes, and add them to nodes to visit if we intersect them
		for (auto& childNode : node->ChildNodes)
		{
			if (childNode->Bounds.DoesRayIntersect(ray, n, f))
			{
				nodesToVisit.push(childNode);
			}
		}

		for(auto& surface : node->ContentsVector)
		{
			//Don't test hitbox here, we alreay did that before pusing it onto the stack
			if (surface->CheckRayOcclusion(ray, maxDistance))
			{
				return true;
			}
		}
	}
	return false;
}

bool Raytracer::RaytraceGetSurface(const Ray & ray, RayHitSurfaceDetails& outSurfaceDetails)
{
	float nearestHit = FLT_MAX;
	bool didHit = false;

	didHit = RaytraceGetSurfaceRecusrive(ray, _sceneBVH, outSurfaceDetails, nearestHit);

	//Fixup hit point
	if(didHit)
	{
		auto rayHitPoint = ray.Origin + ray.Direction * nearestHit;
		outSurfaceDetails.SurfacePosition = rayHitPoint;
	}

	return didHit;
}


bool Raytracer::RaytraceGetSurfaceRecusrive(const Ray& ray, BVHNode<Surface*>* node,
	RayHitSurfaceDetails& outSurfaceDetails, float& outClosestHit)
{
	float near, far;
	if(!node->Bounds.DoesRayIntersect(ray,near,far))
	{
		return false;
	}

	if(near >= outClosestHit)
	{
		//return false;
	}

	bool didHitAny = false;
	if(node->ChildNodes.size() > 0)
	{
		for(auto& child : node->ChildNodes)
		{			
			float thisIntersectionDistance = outClosestHit;
			RayHitSurfaceDetails thisHitSurface;

			if(RaytraceGetSurfaceRecusrive(ray,child, thisHitSurface, thisIntersectionDistance))
			{
				didHitAny = true;
				if(thisIntersectionDistance < outClosestHit)
				{
					outClosestHit = thisIntersectionDistance;
					outSurfaceDetails = thisHitSurface;
				}
			}
		}
	}
	else
	{
		//Foeach surface in node
		for(auto surface : node->ContentsVector)
		{
			RayHitSurfaceDetails localHit;
			float localClosest = outClosestHit;
			if(surface->AttemptRayIntersection(ray, localClosest, localHit))
			{
				didHitAny = true;
				if (localClosest < outClosestHit)
				{
					outClosestHit = localClosest;
					outSurfaceDetails = localHit;
				}
			}
		}
	}
	return didHitAny;
}

bool Raytracer::GetSurfacesFromSceneObject(SceneObject* sceneObject, std::vector<Surface*>* destinationList)
{

	auto surfaceAttempt = sceneObject->TryGetComponent<RenderableSceneComponent>();

	if(surfaceAttempt == nullptr)
	{
		return true;
	}

	Surface* surface;
	bool gotSurface = surfaceAttempt->GetVisableSurfaces(surface);
	if(gotSurface)
	{
		destinationList->push_back(surface);
	}

	return true;
}
