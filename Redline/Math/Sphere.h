#pragma once
#include <mathfu/glsl_mappings.h>
namespace Redline
{
	class Sphere
	{
	public:
		static mathfu::vec3 SampleHemisphere(const mathfu::vec2 & Xi, const mathfu::vec3 & normal);
	};
}