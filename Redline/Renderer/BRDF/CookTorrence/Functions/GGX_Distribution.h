#pragma once

#include <Renderer/BRDF/CookTorrence/DistributionFunction.h>

namespace Redline
{
	namespace CookTorrenceBRDF
	{
		class GGX_Distribution : public DistributionFunction
		{
		public:

			float Distribution(
				const mathfu::vec3& halfAngle,
				const mathfu::vec3& normal,
				float roughness) const override;

			float DistributionProbability(
				const mathfu::vec3& halfAngle,
				const mathfu::vec3& normal,
				const mathfu::vec3& viewAngle, float roughness) const override;

			mathfu::vec3 ImportanceSampleDistribution(
				const mathfu::vec2 & Xi,
				const mathfu::vec3 & normal,
				float roughness) const override;
		};
	}
}