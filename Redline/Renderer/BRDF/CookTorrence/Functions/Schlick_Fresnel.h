#pragma once
#include <Renderer/BRDF/CookTorrence/FresnelFunction.h>

namespace Redline
{
	namespace CookTorrenceBRDF
	{
		class Schlick_Fresnel : public FresnelFunction
		{
		public:
			mathfu::vec3 Reflectance(
				const mathfu::vec3& viewDir,
				const mathfu::vec3& halfAngle,
				const mathfu::vec3& surfaceColor,
				float surfaceMetalness)const override;
		};
	}
}