#pragma once
#include <mathfu/glsl_mappings.h>

namespace Redline
{
	namespace CookTorrenceBRDF
	{
		//This class represents a distribution function (D term) for the Cook-Torrence BRDF
		class DistributionFunction
		{
		public:
			virtual ~DistributionFunction() = default;

			//Compute the percentage of microfacets with a normal of halfAngle for a surface with a given normal and roughness
			virtual float Distribution(
				const mathfu::vec3& halfAngle,
				const mathfu::vec3& normal,
				float roughness) const = 0;

			//Probability distribution function for importance sampling the distribution
			virtual float DistributionProbability(
				const mathfu::vec3& halfAngle,
				const mathfu::vec3& normal,
				const mathfu::vec3& viewAngle, float roughness) const = 0;

			//Given a uniform random value in Xi, compute a microfacet normal for sampling
			//This returns the microfacet normal, and not the incident ray
			virtual mathfu::vec3 ImportanceSampleDistribution(
				const mathfu::vec2 & Xi,
				const mathfu::vec3 & normal,
				float roughness) const = 0;
		};
	}
}