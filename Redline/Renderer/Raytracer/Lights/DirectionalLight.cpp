#include "stdafx.h"
#include "DirectionalLight.h"
#include <Math/Ray.h>
#include "Math/Sampling.h"
#include "Math/Sphere.h"
#include "Utilities/Multithreading/ThreadLocalRandom.h"

using namespace Redline;
using namespace mathfu;

vec3 DirectionalLight::GetRadianceAtPoint(const vec3& point) const
{
	return Color * Power;
}

bool DirectionalLight::GetRadianceAtPointShadowed(const vec3& point, Raytracer& raytracer,
	bool enableSoftShadows, int maxShadowSamples, vec3& outRadiance) const
{
	//Shadows Disabled
	if(maxShadowSamples == 0)
	{
		outRadiance = Color * Power;
		return true;
	}

	auto simulatedLightPosition = point + (-Direction * PhysicalDistance);

	//Only one shadow sample enabled, so take the center
	if(!enableSoftShadows)
	{
		Ray lightVisabilityRay;
		lightVisabilityRay.Origin = point;
		lightVisabilityRay.Direction = -Direction;

		if (raytracer.RaytraceOcclusion(lightVisabilityRay, PhysicalDistance))
		{
			outRadiance = vec3(0.0f, 0.0f, 0.0f);//If our view of the light is blocked, continue to next light to skip shading
			return false;
		}
		outRadiance = Color * Power;
		return true;
	}

	//Else, we are sampling coverage and then returning the light
	//TODO: In the future, we will evaluate the BRDF for each sample.
	//because things need to be more expensive

	vec2 offset(ThreadLocalRandom::NormalFloat(), ThreadLocalRandom::NormalFloat());

	float visability = 0.0f;
	for(int i = 0; i < maxShadowSamples;i++)
	{
		vec2 xi = Sampling::Hammersly2D(i, maxShadowSamples, offset);
		vec3 hemisphereDir = Sphere::SampleHemisphere(xi, Direction);

		vec3 hemispherePoint = simulatedLightPosition + hemisphereDir * PhysicalRadius;
		vec3 surfaceToSphere = hemispherePoint - point;

		Ray lightVisabilityRay;
		lightVisabilityRay.Origin = point;
		lightVisabilityRay.Direction = surfaceToSphere.Normalized();

		if (!raytracer.RaytraceOcclusion(lightVisabilityRay, PhysicalDistance))//No need to compute the real distance, this is just a occlusion test
		{
			visability += 1.0f;
		}
	}

	visability /= maxShadowSamples;

	if(visability < 0.0001f)
	{
		outRadiance = vec3(0.0f, 0.0f, 0.0f);
		return false;
	}

	outRadiance = Color * Power * visability;
	return true;
}

bool DirectionalLight::CanLightAffectBounds(const BoundingBox& bounds) const
{
	return true;
}
