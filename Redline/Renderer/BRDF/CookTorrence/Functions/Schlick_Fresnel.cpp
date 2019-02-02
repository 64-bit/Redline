#include "stdafx.h"
#include "Schlick_Fresnel.h"

using namespace mathfu;
using namespace Redline;
using namespace CookTorrenceBRDF;

vec3 Schlick_Fresnel::Reflectance(
	const mathfu::vec3& viewDir,
	const mathfu::vec3& halfAngle,
	const mathfu::vec3& surfaceColor,
	float surfaceMetalness) const
{
	vec3 F0 = vec3(0.04f, 0.04f, 0.04f);
	F0 = vec3::Lerp(F0, surfaceColor, surfaceMetalness);

	float VoH = vec3::DotProduct(viewDir, halfAngle);
	float pow = powf((1.0f - VoH), 5.0f);

	return F0 + (vec3(1.0f, 1.0f, 1.0f) - F0) * pow;
}
