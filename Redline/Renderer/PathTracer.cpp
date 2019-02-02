#include "stdafx.h"
#include "PathTracer.h"
#include <Renderer/BRDF/BRDF.h>
#include <Renderer/RayLifeDetails.h>
#include <Utilities/Multithreading/ThreadLocalRandom.h>
#include <Math/Sampling.h>
#include <Math/Sphere.h>
#include "CompileTimeSettings.h"

using namespace mathfu;
using namespace Redline;

PathTracer::PathTracer(
	const FrameOutputSettings& frameOutputSettings,
	const FrameQuailtySettings& frameQuailtySettings,
	std::shared_ptr<BRDF> brdf) :
	_outputSettings(frameOutputSettings),
	_quailtySettings(frameQuailtySettings),
	_brdf(brdf)
{

}

void PathTracer::SetSceneData(Raytracer& raytracer, SceneLightingData& sceneLights)
{
	_raytracerInstance = &raytracer;
	_sceneLights = sceneLights;//Intentionaly take copy of this data, so that it can be freed elsewhere
}

vec3 PathTracer::ShadeRayStart(const ViewRay& rayStart)
{
	RayLifeDetails rayLife(_quailtySettings);
	return GetRadianceFromRay(rayStart, rayLife);
}

vec3 PathTracer::GetRadianceFromSurfaceIncident(const RayHitSurfaceDetails& surface, const mathfu::vec3& incidentRay, RayLifeDetails& rayLifeDetails)
{
	Ray ray;
	ray.Direction = incidentRay;
	ray.Origin = surface.SurfacePosition + CompileTimeSettings::NEXT_RAY_SURFACE_OFFSET * incidentRay + CompileTimeSettings::NEXT_RAY_SURFACE_OFFSET * surface.SurfaceNormal_Geometry;

	RayLifeDetails subRayLifeDetails(rayLifeDetails);
	subRayLifeDetails.RemainingSpecularBounces -= 1;
	subRayLifeDetails.RemainingDiffuseBounces -= 1;
	subRayLifeDetails.MipLevelReduction += 10.0f * surface.SurfaceRoughness; //TODO:Make the 4.0f adjustable

	return GetRadianceFromRay(ray, subRayLifeDetails);
}

vec3 PathTracer::GetRadianceFromRay(const Ray& ray, RayLifeDetails& rayLifeDetails)
{
	RayHitSurfaceDetails surfaceDetails;
	bool didHitSurface = _raytracerInstance->RaytraceGetSurface(ray, surfaceDetails);

	//TODO:Add method to environment so we don't do this logic here
	if (!didHitSurface)
	{
		//Evaluate environment and return that
		if (_sceneLights.EnvironmentCubemap != nullptr)
		{
			return _sceneLights.EnvironmentCubemap->SampleCubemap(ray.Direction, rayLifeDetails.MipLevelReduction) * _sceneLights.EnvironmentPower;
		}
		else
		{
			return _sceneLights.BackgroundColor;
		}
	}

	//Else we hit a surface. Shade this surface and reutrn the value
	return ShadeSurface(ray, surfaceDetails, rayLifeDetails);
}

vec3 PathTracer::ShadeLightSourceSpecular_Sampled(const RayHitSurfaceDetails& surface, const vec3& inverseViewRay,
	const vec3& surfaceToLight, const vec3& lightColor)
{
	float NoL = Saturate(vec3::DotProduct(surface.SurfaceNormal, surfaceToLight));
	if (NoL <= 0.0f)
	{
		return vec3(0.0f, 0.0f, 0.0f); //Early out before shadows if this cannot light the surface because of normals
	}

	float probability;
	const vec3 reflectance = _brdf->ComputeReflectanceSpecular(probability, surface, inverseViewRay, surfaceToLight);

	return (reflectance / probability) * lightColor * NoL;
}

vec3 PathTracer::ShadeLightSourceDiffuse_Sampled(const RayHitSurfaceDetails& surface, const vec3& inverseViewRay,
	const vec3& surfaceToLight, const vec3& lightColor)
{
	float NoL = Saturate(vec3::DotProduct(surface.SurfaceNormal, surfaceToLight));
	if (NoL <= 0.0f)
	{
		return vec3(0.0f, 0.0f, 0.0f); //Early out before shadows if this cannot light the surface because of normals
	}

	float probability;
	const vec3 reflectance = _brdf->ComputeReflectanceDiffuse(probability, surface, inverseViewRay, surfaceToLight);
	return (reflectance / probability) * lightColor * NoL;
}

//TODO: This should be able to handle any type of light directly and not be called from 3 different methods
vec3 PathTracer::ShadeLightSourceAnalytical(const RayHitSurfaceDetails& surface, const vec3& inverseViewRay, const vec3& surfaceToLight, const vec3& lightColor)
{
	float NoL = Saturate(vec3::DotProduct(surface.SurfaceNormal, surfaceToLight));

	float probabiliityOut;

	const auto specularReflectance = _brdf->ComputeReflectanceSpecular(probabiliityOut, surface, inverseViewRay, surfaceToLight);
	const auto diffuseReflectance = _brdf->ComputeReflectanceDiffuse(probabiliityOut, surface, inverseViewRay, surfaceToLight);

	return (specularReflectance + diffuseReflectance) * NoL * lightColor;
}

vec3 PathTracer::ShadeSurface(const Ray& viewRay, const RayHitSurfaceDetails& surface, RayLifeDetails& rayLifeDetails)
{
	auto result = vec3(0.0f, 0.0f, 0.0f);

	const auto inverseViewRay = -viewRay.Direction.Normalized();

	auto vOn = vec3::DotProduct(inverseViewRay, surface.SurfaceNormal);
	if (vOn <= 0.0f)
	{
		return result;//Early out for black on wrong faceing surfaces, that we can't see
	}

	//Accumulate lighting from analytical lights
	result += EvaluateAnalyticalLights(surface, inverseViewRay);

	//Accumulate environmental lighting
	result += EvaluateEnvironmentalLights(surface, inverseViewRay, rayLifeDetails);

	//Emissive is rather easy to evaultate
	result += surface.SurfaceEmission;

	return result;
}

vec3 PathTracer::EvaluateAnalyticalLights(const RayHitSurfaceDetails& surface, const vec3& inverseViewRay)
{
	int lightSamples = _quailtySettings.EnableShadows ? _quailtySettings.AnalyticLightSamples : 0;

	vec3 result = vec3(0.0f, 0.0f, 0.0f);

	//Directional lights
	ProcessDirectionalLights(result, surface, inverseViewRay, lightSamples);

	//Point Lights
	ProcessPointLights(result, surface, inverseViewRay, lightSamples);

	//Spot Lights
	ProcessSpotLights(result, surface, inverseViewRay, lightSamples);

	return result;
}

vec3 PathTracer::EvaluateEnvironmentalLights(
	const RayHitSurfaceDetails& surface,
	const vec3& inverseViewRay,
	RayLifeDetails& rayLifeDetails)
{
	vec3 result = vec3(0.0f, 0.0f, 0.0f);
	vec2 offsets = vec2(ThreadLocalRandom::NormalFloat(), ThreadLocalRandom::NormalFloat());

	//TODO: Importance sample between diffuse and specular based on roughness & metalness

	//Environmental Specular
	if (rayLifeDetails.RemainingSpecularBounces > 0)
	{
		vec3 envSpecularColor = vec3(0.0f, 0.0f, 0.0f);

		const unsigned samples = _quailtySettings.SpecularReflectionSamples;
		for (unsigned i = 0; i < samples; i++)
		{
			auto xi = Sampling::Hammersly2D(i, samples, offsets);

			const auto microfacetDirection = _brdf->ImportanceSampleSpecular(
				surface.SurfaceNormal,
				xi,
				surface.SurfaceRoughness * surface.SurfaceRoughness);

			//reflect incomming over microfacet for outgoing
			auto incidentDirection = Reflect(inverseViewRay, microfacetDirection);

			auto envColor = GetRadianceFromSurfaceIncident(surface, incidentDirection, rayLifeDetails);

			auto specular = ShadeLightSourceSpecular_Sampled(surface, inverseViewRay, incidentDirection, envColor);
			envSpecularColor += specular;
		}
		envSpecularColor /= (float)samples;
		result += envSpecularColor;
	}

	//Environmental Diffuse
	if (rayLifeDetails.RemainingDiffuseBounces > 0 && surface.SurfaceMetalness < 0.5f)
	{
		vec3 envDiffuseColor = vec3(0.0f, 0.0f, 0.0f);

		const unsigned samples = _quailtySettings.DiffuseReflectionSamples;
		for (unsigned i = 0; i < samples; i++)
		{
			auto xi = Sampling::Hammersly2D(i, samples, offsets);

			auto incidentDirection = Sphere::SampleHemisphere(xi, surface.SurfaceNormal);

			auto envColor = GetRadianceFromSurfaceIncident(surface, incidentDirection, rayLifeDetails);

			auto diffuse = ShadeLightSourceDiffuse_Sampled(surface, inverseViewRay, incidentDirection, envColor);
			envDiffuseColor += diffuse;
		}

		envDiffuseColor /= (float)samples;
		result += envDiffuseColor;
	}

	return result;
}

void PathTracer::ProcessDirectionalLights(vec3& resultAccumulator, const RayHitSurfaceDetails& surface, const vec3& inverseViewRay, int lightSamples)
{
	for (const auto& directionalLight : _sceneLights.DirectionalLights)
	{
		const auto inverseLightDir = directionalLight.Direction * -1.0f;

		//Early out on NoL
		auto nDl = vec3::DotProduct(surface.SurfaceNormal, inverseLightDir);
		if (nDl <= 0.0f)
		{
			continue;
		}

		vec3 queryPoint = surface.SurfacePosition + CompileTimeSettings::NEXT_RAY_SURFACE_OFFSET * inverseLightDir + CompileTimeSettings::NEXT_RAY_SURFACE_OFFSET * surface.SurfaceNormal_Geometry;

		vec3 lightRadiance;

		//TODO: GetRadianceAtPointShadowed needs to return the real point on the light or the BRDF breaks
		if (directionalLight.GetRadianceAtPointShadowed(queryPoint, *_raytracerInstance, _quailtySettings.EnableSoftShadows, lightSamples, lightRadiance))
		{
			resultAccumulator += ShadeLightSourceAnalytical(
				surface, inverseViewRay, inverseLightDir,
				lightRadiance);
		}
	}
}

void PathTracer::ProcessPointLights(vec3& resultAccumulator, const RayHitSurfaceDetails& surface, const vec3& inverseViewRay, int lightSamples)
{
	for (const auto& pointLight : _sceneLights.PointLights)
	{
		const auto inverseLightDir = (pointLight.Position - surface.SurfacePosition).Normalized();

		//Early out on NoL
		auto nDl = vec3::DotProduct(surface.SurfaceNormal, inverseLightDir);
		if (nDl <= 0.0f)
		{
			continue;
		}

		vec3 queryPoint = surface.SurfacePosition + CompileTimeSettings::NEXT_RAY_SURFACE_OFFSET * inverseLightDir + CompileTimeSettings::NEXT_RAY_SURFACE_OFFSET * surface.SurfaceNormal_Geometry;

		vec3 lightRadiance;
		if (pointLight.GetRadianceAtPointShadowed(queryPoint, *_raytracerInstance, _quailtySettings.EnableSoftShadows, lightSamples, lightRadiance))
		{
			resultAccumulator += ShadeLightSourceAnalytical(
				surface, inverseViewRay, inverseLightDir,
				lightRadiance);
		}
	}
}

void PathTracer::ProcessSpotLights(vec3& resultAccumulator, const RayHitSurfaceDetails& surface, const vec3& inverseViewRay, int lightSamples)
{
	for (const auto& spotLight : _sceneLights.SpotLights)
	{
		const auto inverseLightDir = (spotLight.Position - surface.SurfacePosition).Normalized();

		//Early out on NoL //TODO:This may be too aggressive if the light has a physical size
		auto nDl = vec3::DotProduct(surface.SurfaceNormal, inverseLightDir);
		if (nDl <= 0.0f)
		{
			continue;
		}

		vec3 queryPoint = surface.SurfacePosition + CompileTimeSettings::NEXT_RAY_SURFACE_OFFSET * inverseLightDir + CompileTimeSettings::NEXT_RAY_SURFACE_OFFSET * surface.SurfaceNormal_Geometry;

		vec3 lightRadiance;

		if (spotLight.GetRadianceAtPointShadowed(queryPoint, *_raytracerInstance, _quailtySettings.EnableSoftShadows, lightSamples, lightRadiance))
		{
			resultAccumulator += ShadeLightSourceAnalytical(
				surface, inverseViewRay, inverseLightDir,
				lightRadiance);
		}
	}
}