#pragma once
#include <mathfu/glsl_mappings.h>
#include <Renderer/Raytracer.h>
#include "SceneLightingData.h"
#include "BRDF/BRDF.h"

namespace Redline
{
	struct Ray;
	struct RayLifeDetails;
	struct ViewRay;

	//This class manages the details of path tracing the scene, using the raytracer class to find surfaces,
	//and the BRDF / material settings of that surface to do the estimation of the lighting intergral.
	class PathTracer
	{
	public:

		PathTracer(
			const FrameOutputSettings& frameOuputSettings,
			const FrameQuailtySettings& frameQuailtySettings,
			std::shared_ptr<BRDF> brdf);

		void SetSceneData(Raytracer& raytracer, SceneLightingData& sceneLights);

		//The public API for the path tracer is to get a final color from a ray start.
		//Pretty much by constructing the RayLifeDetails, and then calling GetRadianceFromRay
		mathfu::vec3 ShadeRayStart(const ViewRay& rayStart);

	private:
		FrameOutputSettings _outputSettings;
		FrameQuailtySettings _quailtySettings;
		Raytracer* _raytracerInstance;
		SceneLightingData _sceneLights;
		std::shared_ptr<BRDF> _brdf;//TODO: Move this into the material itself

		//Get the incomming radiance along a given ray. called many times for a given path
		mathfu::vec3 GetRadianceFromRay(const Ray& ray, RayLifeDetails& rayLifeDetails);

		mathfu::vec3 GetRadianceFromSurfaceIncident(const RayHitSurfaceDetails& surface, const mathfu::vec3& incidentRay, RayLifeDetails& rayLifeDetails);

		mathfu::vec3 ShadeSurface(const Ray& viewRay, const RayHitSurfaceDetails& surface, RayLifeDetails& rayLifeDetails);

		mathfu::vec3 EvaluateAnalyticalLights(const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay);

		mathfu::vec3 EvaluateEnvironmentalLights(const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay, RayLifeDetails& rayLifeDetails);

		mathfu::vec3 ShadeLightSourceSpecular_Sampled(const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay,
			const mathfu::vec3& surfaceToLight, const mathfu::vec3& lightColor);

		mathfu::vec3 ShadeLightSourceDiffuse_Sampled(const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay,
			const mathfu::vec3& surfaceToLight, const mathfu::vec3& lightColor);

		mathfu::vec3 ShadeLightSourceAnalytical(const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay, const mathfu::vec3& surfaceToLight, const mathfu::vec3& lightColor);


		void ProcessDirectionalLights(mathfu::vec3& resultAccumulator, const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay, int lightSamples);

		void ProcessPointLights(mathfu::vec3& resultAccumulator, const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay, int lightSamples);

		void ProcessSpotLights(mathfu::vec3& resultAccumulator, const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay, int lightSamples);
	};
}
