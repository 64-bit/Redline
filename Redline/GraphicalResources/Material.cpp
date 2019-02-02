#include "stdafx.h"
#include "Material.h"

using namespace Redline;
using namespace mathfu;


Material::Material()
{
	AlbedoTexture = nullptr;
	NormalMap = nullptr;
	EmissionMap = nullptr;
	MetalnessRoughnessTexture = nullptr;

	Albedo = vec3(0.8f, 0.8f, 0.8f);
	Emission = vec3(0.0f, 0.0f, 0.0f);

	Metalness = 0.0f;
	Roughness = 0.4f;

	AlphaMode = EAlphaMode::OPAQUE;
	AlphaCutoff = 0.5f;
	IsDoubleSided = true;
}

void Redline::Material::ApplyToSurfaceHit(RayHitSurfaceDetails& inOutSurfaceDetails, const mathfu::vec2& uv, float mipmapLevel)
{
	if(AlbedoTexture != nullptr)
	{
		inOutSurfaceDetails.SurfaceAlbedo = Albedo * AlbedoTexture->SampleTextureBilinear(uv,(unsigned)mipmapLevel);
	}
	else
	{
		inOutSurfaceDetails.SurfaceAlbedo = Albedo;
	}

	if(EmissionMap != nullptr)
	{
		inOutSurfaceDetails.SurfaceEmission = Emission * EmissionMap->SampleTextureBilinear(uv,(unsigned)mipmapLevel);
	}
	else
	{
		inOutSurfaceDetails.SurfaceEmission = Emission;
	}

	if(MetalnessRoughnessTexture != nullptr)
	{
		vec3 sample = MetalnessRoughnessTexture->SampleTextureBilinear(uv, (unsigned)mipmapLevel);
		inOutSurfaceDetails.SurfaceMetalness = Metalness * sample.x;
		inOutSurfaceDetails.SurfaceRoughness = Roughness * sample.y;
	}
	else
	{
		inOutSurfaceDetails.SurfaceMetalness = Metalness;
		inOutSurfaceDetails.SurfaceRoughness = Roughness;
	}

	if(NormalMap != nullptr)
	{
		vec3 normalSample = NormalMap->SampleTextureBilinear(uv, (unsigned)mipmapLevel);
		normalSample *= 2.0f;
		normalSample = normalSample - vec3(1.0f, 1.0f, 1.0f);

		normalSample.Normalize();
		//Modify surface normal based on map
		vec3 originalNormal = inOutSurfaceDetails.SurfaceNormal;
		vec3 endNormal = inOutSurfaceDetails.TangentMatrix * normalSample;
		inOutSurfaceDetails.SurfaceNormal = endNormal.Normalized();
	}
}
