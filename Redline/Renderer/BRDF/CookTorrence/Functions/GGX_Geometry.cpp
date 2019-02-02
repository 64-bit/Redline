#include "stdafx.h"
#include "GGX_Geometry.h"

using namespace mathfu;
using namespace Redline;
using namespace CookTorrenceBRDF;

float GGX_Geometry::Geometry(
	const vec3& incommingAngle,
	const vec3& outgoingAngle,
	const vec3& normal,
	const vec3& halfAngle,
	float roughness) const
{
	const float incomming = GGX_Geometry_Partial(
		incommingAngle,
		normal,
		halfAngle,
		roughness
	);

	const float outgoing = GGX_Geometry_Partial(
		outgoingAngle,
		normal,
		halfAngle,
		roughness
	);

	return incomming * outgoing;
}

float GGX_Geometry::GGX_Geometry_Partial(
	const vec3& incidentAngle,
	const vec3& normal,
	const vec3& halfAngle,
	float roughness) const
{
	const float VoH = Saturate(vec3::DotProduct(incidentAngle, halfAngle));
	const float VoN = Saturate(vec3::DotProduct(incidentAngle, normal));
	const float chi = (VoH / VoN);

	if(chi <= 0.0f)
	{
		return 0.0f;
	}

	const float VoH2 = VoH * VoH;
	const float a2 = roughness * roughness;


	const float tan2 = (1.0f - VoH2) / VoH2;

	const float num = 2.0f * VoN;
	const float denom = (VoN + sqrt(a2 + (1.0f - a2) * VoN*VoN));

	return Saturate(num / denom);
}