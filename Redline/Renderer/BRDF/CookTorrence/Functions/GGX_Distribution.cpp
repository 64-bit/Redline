#include <stdafx.h>
#include "GGX_Distribution.h"

using namespace Redline;
using namespace CookTorrenceBRDF;
using namespace mathfu;

float GGX_Distribution::Distribution(const vec3& halfAngle,
	const vec3& normal, float roughness) const
{
	const float NoH = vec3::DotProduct(normal, halfAngle);
	if (NoH <= 0.0f)
	{
		return 0.0f;
	}

	const float r2 = roughness * roughness;
	const float NoH2 = NoH * NoH;

	const float num = r2;
	const float den = NoH2 * (r2 - 1.0f) + 1.0f;

	return (num / (static_cast<float>(M_PI) * den * den));
}

float GGX_Distribution::DistributionProbability(const vec3& halfAngle,
	const vec3& normal, const vec3& viewAngle, float roughness) const
{
	const float ggx = Distribution(halfAngle, normal, roughness);
	const float num = ggx * vec3::DotProduct(normal, halfAngle);
	const float denom = 4.0f * vec3::DotProduct(halfAngle, viewAngle);

	return num / denom;
}

vec3 GGX_Distribution::ImportanceSampleDistribution(const vec2& Xi,
	const vec3& normal, float roughness) const
{
	float a2 = roughness * roughness;
	//GGX is isotropic, so create a normal with a evenly distributed radial angle
	float Phi = 2.0f * (float)M_PI * Xi.x;

	//use roughness to determin the vertical angle of the microfacet
	float num = 1.0f - Xi.y;
	float denom = Xi.y * (a2 - 1.0f) + 1.0f;
	float sqareRoot = sqrt(num / denom);

	float theta = acos(sqareRoot);

	//Polar to cartesian transform
	float sinTheta = sin(theta);
	float cosTheta = cos(theta);

	vec3 tangentSpaceMicrofacetNormal;
	tangentSpaceMicrofacetNormal.x = sinTheta * cos(Phi);
	tangentSpaceMicrofacetNormal.y = sinTheta * sin(Phi);
	tangentSpaceMicrofacetNormal.z = cosTheta;

	// Tangent to world space basis vectors
	vec3 UpVector = abs(normal.y) < 0.999999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
	vec3 TangentX = normalize(cross(UpVector, normal));
	vec3 TangentY = cross(normal, TangentX);

	//multiply microfacet normal by basis vectors
	return TangentX * tangentSpaceMicrofacetNormal.x 
	+ TangentY * tangentSpaceMicrofacetNormal.y 
	+ normal * tangentSpaceMicrofacetNormal.z;
}