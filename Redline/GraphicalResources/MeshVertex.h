#pragma once
#include "mathfu/glsl_mappings.h"

namespace Redline
{
	struct MeshVertex
	{
		mathfu::vec3 Position;
		mathfu::vec2 UV;
		mathfu::vec3 Normal;
		mathfu::vec3 BiNormal;
		mathfu::vec3 Tangent;

	};
}