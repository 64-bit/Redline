#pragma once

#include "FrameOutputSettings.h"
#include "../Scene/Scene.h"
#include <memory>
#include "Raytracer/Surfaces/Surface.h"

namespace Redline
{
	template<typename T>
	class BVHNode;
	/*
	 * Raytracer class
	 * 
	 * Responsible for building scene accelration structure from graphical / file scene
	 * Responsible for tracing rays in the scene and returning what they hit
	 */
	class Raytracer
	{
	public:
		//Extracts all meshes from the scene and builds the accleration structure
		bool BuildSceneStructure(const std::shared_ptr<Scene>& scene);

		bool GetIsReady() const;

		//Raytrace the scene testing for occlusion. The ray will only be tested to a length of maxDistance
		//Returns true if the ray hits anything, false otherwise
		bool RaytraceOcclusion(const Ray& ray, float maxDistance);

		//Raytrace the scene and return information about the first surface the ray hits
		//Returns true if the ray hits anything, false otherwise.
		//The output paramater outSurfaceDetails is only valid after this method returns true, otherwise it is undefined
		bool RaytraceGetSurface(const Ray& ray, RayHitSurfaceDetails& outSurfaceDetails);

	private:
		
		bool _isReady = false; //Indicates all structures are ready and the raytracer can trace the scene.

		std::vector<Surface*> _surfaces;
		BVHNode<Surface*>* _sceneBVH;
		std::shared_ptr<Scene> _scene;

		bool RaytraceGetSurfaceRecusrive(const Ray& ray, BVHNode<Surface*>* node, RayHitSurfaceDetails& outSurfaceDetails, float& outClosestHit);

		static bool GetSurfacesFromSceneObject(SceneObject* sceneObject, std::vector<Surface*>* destinationList);
	};
}
