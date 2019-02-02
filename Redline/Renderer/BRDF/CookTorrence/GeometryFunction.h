#pragma once
#include <mathfu/glsl_mappings.h>

namespace Redline
{
	namespace CookTorrenceBRDF
	{
		//This class represents a geometry function (G term) for the Cook-Torrence BRDF
		class GeometryFunction
		{
		public:
			//Compute the geometry self shadowing term for cook-torrence
			virtual float Geometry(
				const mathfu::vec3& incommingAngle,
				const mathfu::vec3& outgoingAngle,
				const mathfu::vec3& normal,
				const mathfu::vec3& halfAngle,
				float roughness) const =0;
		};
	}
}