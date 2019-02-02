#pragma once
#include "mathfu/glsl_mappings.h"
#include "../Renderer/Raytracer/RayHitSurfaceDetails.h"
#include "Bitmap2D.h"
#include "MaterialEnums.h"

namespace Redline
{
	//Represents a surface material.
	//In it's current state, the render only supports simple PBR materials of a single type.
	class Material
	{
	public:
		Material();

		mathfu::vec3 Albedo;
		std::shared_ptr<Bitmap2D> AlbedoTexture;
		std::shared_ptr<Bitmap2D> NormalMap;

		mathfu::vec3 Emission;
		std::shared_ptr<Bitmap2D> EmissionMap;

		//Possibly don't use this, as we acutaly simulate this by sampling rays
		//But without it, we won't have AO on normals, so let's keep it and see how it looks.
		//We can always weaken it's effect
		std::shared_ptr<Bitmap2D> AmbientOcclusionMap;

		float Metalness = 0.0f;
		float Roughness = 0.5f;
		std::shared_ptr<Bitmap2D> MetalnessRoughnessTexture;

		bool IsDoubleSided;
		EAlphaMode AlphaMode;
		float AlphaCutoff;

		void ApplyToSurfaceHit(RayHitSurfaceDetails& inOutSurfaceDetails, const mathfu::vec2& uv, float mipmapLevel = 0.0f);
	};
}
