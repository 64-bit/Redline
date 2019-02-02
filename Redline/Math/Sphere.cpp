#include "stdafx.h"
#include "Sphere.h"

using namespace Redline;
using namespace mathfu;

vec3 Sphere::SampleHemisphere(const vec2 & Xi, const vec3 & normal)
{
	float x = 2.0f * (float)M_PI * Xi.x;
	float y = 0.5f * (float)M_PI * Xi.y;

	float sinY = sinf(y);

	vec3 H;
	H.x = sinY * cosf(x);
	H.y = sinY * sinf(x);
	H.z = cosf(y);
	vec3 UpVector = abs(normal.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
	vec3 TangentX = normalize(cross(UpVector, normal));
	vec3 TangentY = cross(normal, TangentX);
	// Tangent to world space
	return (TangentX * H.x + TangentY * H.y + normal * H.z).Normalized();
}
