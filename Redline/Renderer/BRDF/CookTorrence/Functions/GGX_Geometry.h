#pragma once
#include <Renderer/BRDF/CookTorrence/GeometryFunction.h>

namespace Redline
{
	namespace CookTorrenceBRDF
	{
		//Implementation of GGX Geometry function for Cook-Torrance
		class GGX_Geometry : public GeometryFunction
		{
		public:
			float Geometry(
				const mathfu::vec3& incommingAngle,
				const mathfu::vec3& outgoingAngle,
				const mathfu::vec3& normal,
				const mathfu::vec3& halfAngle,
				float roughness) const override;

		private:
			float GGX_Geometry_Partial(
				const mathfu::vec3& incidentAngle,
				const mathfu::vec3& normal,
				const mathfu::vec3& halfAngle,
				float roughness) const;
		};
	}
}