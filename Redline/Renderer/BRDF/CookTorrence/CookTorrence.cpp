#include "stdafx.h"
#include "CookTorrence.h"
#include <Math/Math.h>
#include "Math/Sphere.h"
#include "DistributionFunction.h"
#include "GeometryFunction.h"
#include "FresnelFunction.h"

using namespace mathfu;
using namespace std;

using namespace Redline;
using namespace CookTorrenceBRDF;

CookTorrence::CookTorrence(
	shared_ptr<DistributionFunction> distributionFunction,
	shared_ptr<GeometryFunction> geometryFunction,
	std::shared_ptr<FresnelFunction> fresnelFunction)
	:BRDF()
{
	_distributionFunction = distributionFunction;
	_geometryFunction = geometryFunction;
	_fresnelFunction = fresnelFunction;
}


vec3 CookTorrence::ImportanceSampleSpecular(const vec3& surfaceNormal, const vec2& uniformRandom, float roughness)
{
	return _distributionFunction->ImportanceSampleDistribution(uniformRandom, surfaceNormal, roughness);
}

vec3 CookTorrence::ComputeReflectanceSpecular(float& outProbability, const RayHitSurfaceDetails& surface, const vec3& inverseViewRay,
	const vec3& surfaceToLight)
{
	float NoL = Saturate(vec3::DotProduct(surface.SurfaceNormal, surfaceToLight));
	if (NoL <= 0.0f)
	{
		return vec3(0.0f, 0.0f, 0.0f); //Early out before shadows if this cannot light the surface because of normals
	}

	const auto halfAngle = (inverseViewRay + surfaceToLight).Normalized();

	if (vec3::DotProduct(halfAngle, surfaceToLight) <= 0.0f)
	{
		return vec3(0.0f, 0.0f, 0.0f); //Early out before shadows if this cannot light the surface because of normals
	}

	const float roughness2 = surface.SurfaceRoughness;// *surface.SurfaceRoughness;

	//Compute probability
	outProbability = _distributionFunction->DistributionProbability(
		halfAngle,
		surface.SurfaceNormal,
		inverseViewRay, roughness2);
	//Compute reflectance


	const float distribution = _distributionFunction->Distribution(halfAngle, surface.SurfaceNormal, roughness2);

	const float geometry = _geometryFunction->Geometry(
		inverseViewRay,
		surfaceToLight,
		surface.SurfaceNormal,
		halfAngle,
		surface.SurfaceRoughness);

	const auto fresnelReflectance = _fresnelFunction->Reflectance(
		inverseViewRay,
		halfAngle,
		surface.SurfaceAlbedo,
		surface.SurfaceMetalness);

	const float NoV = Saturate(vec3::DotProduct(surface.SurfaceNormal, inverseViewRay));

	const float num = (distribution * geometry);
	const float denom = 4.0f * NoL * NoV;

	auto reflectance = (num / denom) * fresnelReflectance;
	return reflectance;
}

vec3 CookTorrence::ComputeReflectanceDiffuse(float& outProbability, const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay,
	const mathfu::vec3& surfaceToLight)
{
	const auto halfAngle = (inverseViewRay + surfaceToLight).Normalized();
	float NoL = vec3::DotProduct(surface.SurfaceNormal, surfaceToLight);

	if (NoL <= 0.0f)
	{
		return vec3(0.0f, 0.0f, 0.0f);
	}

	//Compute probability
	float cosT = Saturate(vec3::DotProduct(surfaceToLight, surface.SurfaceNormal));
	float sinT = sqrt(1.0f - cosT * cosT);
	outProbability = 1.0f / sinT;
	//Compute reflectance

	const auto fresnelReflectance = _fresnelFunction->Reflectance(
		inverseViewRay,
		halfAngle,
		surface.SurfaceAlbedo,
		surface.SurfaceMetalness);

	const auto reflectanceFactor = (vec3(1.0f, 1.0f, 1.0f) - fresnelReflectance) * (1.0f - surface.SurfaceMetalness);

	vec3 diffuseColor = surface.SurfaceAlbedo * reflectanceFactor;
	return diffuseColor;
}