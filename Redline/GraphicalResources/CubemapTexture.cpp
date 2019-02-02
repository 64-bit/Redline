#include "stdafx.h"
#include "CubemapTexture.h"

using namespace Redline;
using namespace mathfu;

vec3 CubemapTexture::SampleCubemap(const vec3& vector, float mipmapLevel)
{
	auto normalized = vector.Normalized();
	auto abs = normalized;

	abs.x = std::abs(abs.x);
	abs.y = std::abs(abs.y);
	abs.z = std::abs(abs.z);

	unsigned face;
	vec2 uv;
	float scale;

	if(abs.z >= abs.x && abs.z >= abs.y)
	{
		//One of the z faces
		face = (vector.z > 0.0f) ? 5 : 4;
		uv = vec2(
			vector.z < 0.0f ? -vector.x : vector.x,
			-vector.y);
		scale = 0.5f / abs.z;
	}
	else if(abs.y >= abs.x)
	{
		//one of the y faces
		face = (vector.y > 0.0f) ? 3 : 2;
		uv = vec2(
			vector.x,
			vector.y < 0.0f ? -vector.z : vector.z);
		scale = 0.5f / abs.y;
	}
	else
	{
		//it's the x face
		face = (vector.x > 0.0f) ? 1 : 0;
		uv = vec2(
			vector.x < 0.0f ? vector.z : -vector.z,
			-vector.y);
		scale = 0.5f / abs.x;
	}

	auto finalUV = (uv * scale) + 0.5f;
	if(mipmapLevel > 1.0f)
	{
		int x = 3;
		x += 1;
	}
	return Faces[face]->SampleTextureTrilinear(finalUV, mipmapLevel);
}
