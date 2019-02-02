#pragma once
#include <mathfu/glsl_mappings.h>

namespace Redline
{
	struct RayHitSurfaceDetails
	{
		mathfu::vec3 SurfacePosition;
		mathfu::vec3 SurfaceAlbedo;
		mathfu::vec3 SurfaceNormal;

		mathfu::vec3 SurfaceNormal_Geometry;

		//Surface tangent

		mathfu::mat3 TangentMatrix;

		mathfu::vec3 SurfaceEmission;

		float SurfaceMetalness;
		float SurfaceRoughness;
	};
}
