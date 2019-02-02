#include "stdafx.h"
#include "SpotLight.h"
#include "Utilities/Multithreading/ThreadLocalRandom.h"
#include "Math/Sampling.h"
#include "Math/Sphere.h"
#include <Math/Ray.h>

using namespace Redline;
using namespace mathfu;

float SpotLight::GetFalloff(float distance, float cosTheta) const
{
	float num = (distance / LightRadius);
	num = num * num;
	num = num * num;
	num = mathfu::Clamp((1.0f - num), 0.0f, 1.0f);

	num = num * num;

	float denom = distance * distance + 1.0f;

	float distanceFalloff = num / denom;

	float angleFalloff;

	if(cosTheta >= InnerAngleCos)
	{
		angleFalloff = 1.0f;
	}
	else if(cosTheta < OuterAngleCos)
	{
		angleFalloff = 0.0f;
	}
	else
	{
		angleFalloff = UnLerp(InnerAngleCos, OuterAngleCos, cosTheta);
		angleFalloff = Saturate(angleFalloff);
		angleFalloff = 1.0f - angleFalloff;
	}

	return distanceFalloff * angleFalloff;
}

vec3 SpotLight::GetRadianceAtPoint(const vec3& point) const
{
	float distance = vec3::Distance(Position, point);
	vec3 pointToLight = (Position - point).Normalized();

	float cosTheta = vec3::DotProduct(Direction, pointToLight);

	return GetRadianceAtDistance(distance, cosTheta);
}

vec3 SpotLight::GetRadianceAtDistance(float distance, float cosTheta) const
{
	float falloff = GetFalloff(distance, cosTheta);
	return Color * Power * falloff;
}

bool SpotLight::GetRadianceAtPointShadowed(const vec3& point, Raytracer& raytracer,
	bool enableSoftShadows, int maxShadowSamples, vec3& outRadiance) const
{
	vec3 surfaceToLight = Position - point;

	//Shadows Disabled
	if (maxShadowSamples == 0)
	{
		outRadiance = GetRadianceAtPoint(point);
		return true;
	}

	//No soft shadow, use a single sample of the center of this light
	if (!enableSoftShadows)
	{
		Ray lightVisabilityRay;
		lightVisabilityRay.Origin = point; //TODO:Enable some sort of real test to stop self occlusion
		lightVisabilityRay.Direction = surfaceToLight.Normalized();


		if (raytracer.RaytraceOcclusion(lightVisabilityRay, surfaceToLight.Length()))
		{
			outRadiance = vec3(0.0f, 0.0f, 0.0f);//If our view of the light is blocked, continue to next light to skip shading
			return false;
		}
		else
		{
			outRadiance  = GetRadianceAtPoint(point);
			return true;
		}
	}

	//Else, we are sampling coverage and then returning the light
	//TODO: In the future, we will evaluate the BRDF for each sample, as this is not accurate
	vec2 offset(ThreadLocalRandom::NormalFloat(), ThreadLocalRandom::NormalFloat());

	vec3 lightToSurface = -surfaceToLight.Normalized();

	float visability = 0.0f;
	vec3 radiance = vec3(0.0f, 0.0f, 0.0f);//We use this, so that we can accumlate accurate light falloff

	for (int i = 0; i < maxShadowSamples; i++)
	{
		vec2 xi = Sampling::Hammersly2D(i, maxShadowSamples, offset);
		vec3 hemisphereDir = Sphere::SampleHemisphere(xi, lightToSurface);

		vec3 hemispherePoint = Position + hemisphereDir * PhysicalRadius;
		vec3 surfaceToSphere = hemispherePoint - point;

		float distanceToLightSurface = surfaceToSphere.Length();
		surfaceToSphere.Normalize();

		float cosTheta = vec3::DotProduct(surfaceToSphere, Direction);

		Ray lightVisabilityRay;
		lightVisabilityRay.Origin = point;
		lightVisabilityRay.Direction = surfaceToSphere;

		if (!raytracer.RaytraceOcclusion(lightVisabilityRay, distanceToLightSurface))//No need to compute the real distance, this is just a occlusion test
		{
			visability += 1.0f;
			radiance += GetRadianceAtDistance(distanceToLightSurface, cosTheta);
		}
	}

	outRadiance = radiance / static_cast<float>(maxShadowSamples);
	return visability > 0.0f;

}

bool Redline::SpotLight::CanLightAffectBounds(const BoundingBox& bounds) const
{
	//TODO: check if this cone intersects the bounds, as this is far more expensive than it needs to be
	float boundsRadius = vec3::Distance(bounds.GetCenter(), bounds.Max);
	float distanceToCenter = vec3::Distance(bounds.GetCenter(), Position);

	return boundsRadius + LightRadius > distanceToCenter;
}
