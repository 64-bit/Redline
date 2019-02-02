#include "stdafx.h"
#include "PhongBRDF.h"

using namespace Redline;
using namespace mathfu;

const float NEXT_RAY_SURFACE_OFFSET = 0.00001f;

PhongBRDF::PhongBRDF(const FrameOutputSettings& frameOuputSettings, const FrameQuailtySettings& frameQuailtySettings)
	:BRDF()
{
}

vec3 PhongBRDF::GetColorFromRay(const Ray& ray, unsigned remainingReflectionBounces)
{
	/*RayHitSurfaceDetails surfaceDetails;
	bool didHitSurface = RaytracerInstance->RaytraceGetSurface(ray, surfaceDetails);

	if(!didHitSurface)
	{
		//Evaluate environment and return that
		if(SceneLights.EnvironmentCubemap != nullptr)
		{
			return SceneLights.EnvironmentCubemap->SampleCubemap(ray.Direction);
		}
		else
		{
			return SceneLights.BackgroundColor;
		}
	}

	//Else we hit a surface. Shade this surface and reutrn the value
	return ShadeSurface(ray, surfaceDetails, remainingReflectionBounces);*/
	return vec3(0, 0, 0);
}

vec3 PhongBRDF::ShadeSurface(const Ray& viewRay, const RayHitSurfaceDetails& surface, unsigned remainingReflectionBounces)
{
	/*vec3 lightAccumulation = vec3(0.0f, 0.0f, 0.0f);

	//Ambient light
	lightAccumulation += surface.SurfaceAlbedo * SceneLights.AmbientLight;

	//Directional lights
	for (const auto& directionalLight : SceneLights.DirectionalLights)
	{
		//Shade directional lights
		auto inverseLightDir = directionalLight.Direction * -1.0f;

		auto nDl = vec3::DotProduct(surface.SurfaceNormal, inverseLightDir);
		if (nDl <= 0.0f)
		{
			continue; //Early out before shadows if this cannot light the surface because of normals
		}

		//If shadows are enabled, test shadow intersections
		if (QuailtySettings.EnableShadows)//TODO:Pass in shadow settings
		{
			Ray lightVisabilityRay;
			lightVisabilityRay.Origin = surface.SurfacePosition + NEXT_RAY_SURFACE_OFFSET * -directionalLight.Direction; //TODO:Enable some sort of real test to stop self occlusion
			lightVisabilityRay.Direction = -directionalLight.Direction;

			if(RaytracerInstance->RaytraceOcclusion(lightVisabilityRay, 100000.0f))
			{
				continue;//If our view of the light is blocked, continue to next light to skip shading
			}
		}

		//compute diffuse term
		auto diffuse = nDl * directionalLight.Color * directionalLight.Power * surface.SurfaceAlbedo;

		//Compute specular term
		float exponent = 20.0f + surface.SurfaceRoughness * 500.0f;
		vec3 halfAngle = (-viewRay.Direction + inverseLightDir).Normalized();

		float nDotH = vec3::DotProduct(surface.SurfaceNormal, halfAngle);
		nDotH = Clamp(nDotH, 0.0f, 1.0f);
		float intensity = powf(nDotH, exponent);

		auto specular = intensity * directionalLight.Color;

		lightAccumulation += diffuse;
		lightAccumulation += specular;*/
	//}

	//Point lights
	/*for(const auto& pointLight : SceneLights.PointLights)
	{
		const float lightDistance = vec3::Distance(pointLight.Position, surface.SurfacePosition);
		//test distance and early out if we are above it
		if(lightDistance > pointLight.LightRadius)
		{
			continue;
		}

		//Shade point light lights
		auto inverseLightDir = (pointLight.Position - surface.SurfacePosition).Normalized();

		auto nDl = vec3::DotProduct(surface.SurfaceNormal, inverseLightDir);
		if (nDl <= 0.0f)
		{
			continue; //Early out before shadows if this cannot light the surface because of normals
		}

		//If shadows are enabled, test shadow intersections
		if (QuailtySettings.EnableShadows)//TODO:Pass in shadow settings
		{
			Ray lightVisabilityRay;
			lightVisabilityRay.Origin = surface.SurfacePosition + NEXT_RAY_SURFACE_OFFSET * inverseLightDir; //TODO:Enable some sort of real test to stop self occlusion
			lightVisabilityRay.Direction = inverseLightDir;

			if (RaytracerInstance->RaytraceOcclusion(lightVisabilityRay, lightDistance))
			{
				continue;//If our view of the light is blocked, continue to next light to skip shading
			}
		}

		//compute diffuse term
		auto diffuse = nDl * pointLight.Color * pointLight.Power * pointLight.GetFalloff(lightDistance) * surface.SurfaceAlbedo;

		//Compute specular term
		float exponent = 20.0f + surface.SurfaceRoughness * 500.0f;
		vec3 halfAngle = (-viewRay.Direction + inverseLightDir).Normalized();

		float nDotH = vec3::DotProduct(surface.SurfaceNormal, halfAngle);
		nDotH = Clamp(nDotH, 0.0f, 1.0f);
		float intensity = powf(nDotH, exponent);

		auto specular = intensity * pointLight.Color;

		lightAccumulation += diffuse;
		lightAccumulation += specular;
	}

	//Mirror reflection if necessary
	if (surface.SurfaceMetalness > 0.01f && remainingReflectionBounces > 0)
	{
		Ray mirrorRay;
		mirrorRay.Direction = viewRay.Direction - 2.0f * vec3::DotProduct(viewRay.Direction, surface.SurfaceNormal) * surface.SurfaceNormal;
		mirrorRay.Direction *= 1.0f;

		mirrorRay.Origin = surface.SurfacePosition + mirrorRay.Direction * NEXT_RAY_SURFACE_OFFSET * 2.0f;

		//Shade mirror ray
		auto mirrorColor = GetColorFromRay(mirrorRay, remainingReflectionBounces - 1);
		//Lerp between mirror color and surface color
		lightAccumulation = vec3::Lerp(lightAccumulation, mirrorColor, surface.SurfaceMetalness);
	}
	*/

	return vec3(0,0,0);
}
