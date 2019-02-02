#pragma once
#include <mathfu/glsl_mappings.h>

namespace Redline
{
	struct Color
	{
		unsigned char R;
		unsigned char G;
		unsigned char B;
		unsigned char A;

		Color();
		Color(const mathfu::vec4& floatColor);
		mathfu::vec4 ToVec4() const;
	};

	inline Color::Color()
	{
		R = 0;
		G = 0;
		B = 0;
		A = 0;
	}
	//For some reason is BRG right now
	inline Color::Color(const mathfu::vec4& floatColor)
	{
		mathfu::vec4 clampedColor;

		clampedColor.x = std::max(std::min(1.0f, floatColor.x), 0.0f);
		clampedColor.y = std::max(std::min(1.0f, floatColor.y), 0.0f);
		clampedColor.z = std::max(std::min(1.0f, floatColor.z), 0.0f);
		clampedColor.w = std::max(std::min(1.0f, floatColor.w), 0.0f);

		R = (unsigned char)roundf(clampedColor.x * 255.0f);
		G = (unsigned char)roundf(clampedColor.y * 255.0f);
		B = (unsigned char)roundf(clampedColor.z * 255.0f);
		A = (unsigned char)roundf(clampedColor.w * 255.0f);
	}

	inline mathfu::vec4 Color::ToVec4() const
	{
		return mathfu::vec4(
			(float)R / 255.0f,
			(float)G / 255.0f,
			(float)B / 255.0f,
			(float)A / 255.0f);
	}
}
