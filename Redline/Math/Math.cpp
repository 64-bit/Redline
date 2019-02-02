#include "stdafx.h"
#include "Math.h"

using namespace mathfu;

float Saturate(float val)
{
	if (val < 0.0f)
	{
		return 0.0f;
	}
	if (val > 1.0f)
	{
		return 1.0f;
	}
	return val;
}

vec3 Reflect(const vec3& surfaceToViewer, const vec3 & surfaceNormal)
{
	//return surfaceToViewer - 2.0f * vec3::DotProduct(surfaceToViewer, surfaceNormal) * surfaceNormal;
	return (2.0f * vec3::DotProduct(surfaceToViewer, surfaceNormal) * surfaceNormal - surfaceToViewer).Normalized();
}

float UnLerp(const float a, const float b, const float val)
{
	const auto dist = b - a;
	const auto aToVal = val - a;
	return aToVal / dist;
}

float CalculateLuminance(const vec4 & color)
{
	float r = color.x * color.x * PERCEIVED_BRIGHTNESS_RED;
	float g = color.y * color.y * PERCEIVED_BRIGHTNESS_GREEN;
	float b = color.z * color.z * PERCEIVED_BRIGHTNESS_BLUE;

	return  sqrtf(r + g + b);
}

//See this article
//http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
vec3 ExtractPitchYawRoll(const quat& rotation)
{
	const auto x = rotation[1];
	const auto y = rotation[2];
	const auto z = rotation[3];
	const auto w = rotation[0];

	const auto x2 = x * x;
	const auto y2 = y * y;
	const auto z2 = z * z;


	const auto roll = asinf(2.0f * x * y + 2.0f * z * w);

	const auto yaw = atan2f(
		2.0f * y * w - 2.0f * x * z,
		1.0f - 2.0f * y2 - 2.0f * z2);

	const auto pitch = atan2f(
		2.0f * x * w - 2.0f * y * z,
		1.0f - 2.0f * x2 - 2.0f * z2);

	return mathfu::vec3(pitch, yaw, roll);
}

void DecomposeMatrix(const mat4& matrix,
	vec3& outPosition,
	quat& outRotation,
	vec3& outScale)
{
	//Extract position from matrix
	outPosition[0] = matrix[12];
	outPosition[1] = matrix[13];
	outPosition[2] = matrix[14];

	//Convert matrix4 to matrix 3 for ease of use
	mat3 rotation;
	for (auto i = 0; i < 9; i++)
	{
		const auto row = i / 3;
		const auto col = i % 3;

		rotation[i] = matrix[col + row * 4];
	}

	for (auto i = 0; i < 3; i++)
	{
		//extract scale first, as we need it to fix the rotation matrix
		const auto scale = rotation.GetColumn(i).Length();
		outScale[i] = scale;
		//Remove scale from that collumn
		rotation.GetColumn(i) /= scale;
	}

	//Convert matrix to quaternion
	outRotation = quat::FromMatrix(rotation);
}
