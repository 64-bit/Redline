#pragma once
#include "Bitmap2D.h"
#include "mathfu/glsl_mappings.h"

namespace Redline
{
	class CubemapTexture
	{
		
	public:	
		std::shared_ptr<Bitmap2D> Faces[6];

		mathfu::vec3 SampleCubemap(const mathfu::vec3& vector, float mipmapLevel = 0.0f);
	};
}
