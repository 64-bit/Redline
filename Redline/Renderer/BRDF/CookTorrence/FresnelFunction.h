#pragma once
#include <mathfu/glsl_mappings.h>

namespace Redline
{
	namespace CookTorrenceBRDF
	{
		//This class represents a fresnel function (F term) for the Cook-Torrence BRDF
		class FresnelFunction
		{
		public:
			//TODO: Add support for custom IoR for surface and air

			//Compute fresnel reflectance from material and surface details
			virtual mathfu::vec3 Reflectance(
				const mathfu::vec3& viewDir,
				const mathfu::vec3& halfAngle,
				const mathfu::vec3& surfaceColor,
				float surfaceMetalness)const = 0;
		};
	}
}