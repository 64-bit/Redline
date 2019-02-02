#pragma once
#include "../BRDF.h"

namespace Redline
{
	struct RayLifeDetails;

	namespace CookTorrenceBRDF
	{
		class DistributionFunction;
		class GeometryFunction;
		class FresnelFunction;
	}

	class CookTorrence : public BRDF
	{
	public:
		CookTorrence(
			std::shared_ptr<CookTorrenceBRDF::DistributionFunction> distributionFunction,
			std::shared_ptr<CookTorrenceBRDF::GeometryFunction> geometryFunction,
			std::shared_ptr<CookTorrenceBRDF::FresnelFunction> fresnelFunction);

		//New Methods

		mathfu::vec3 ImportanceSampleSpecular(const mathfu::vec3& surfaceNormal, const mathfu::vec2& uniformRandom, float roughness) override;

		mathfu::vec3 ComputeReflectanceSpecular(float& outProbability, const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay,
			const mathfu::vec3& surfaceToLight) override;

		mathfu::vec3 ComputeReflectanceDiffuse(float& outProbability, const RayHitSurfaceDetails& surface, const mathfu::vec3& inverseViewRay,
			const mathfu::vec3& surfaceToLight) override;

	private:
		std::shared_ptr<CookTorrenceBRDF::DistributionFunction> _distributionFunction;
		std::shared_ptr<CookTorrenceBRDF::GeometryFunction> _geometryFunction;
		std::shared_ptr<CookTorrenceBRDF::FresnelFunction> _fresnelFunction;

		static mathfu::vec3 ImportanceSampleDiffuse(const mathfu::vec2& Xi, const mathfu::vec3& normal);
	};
}
