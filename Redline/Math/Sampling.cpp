#include "stdafx.h"
#include "Sampling.h"

using namespace Redline;
using namespace mathfu;

float radicalInverse_VdC(unsigned bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return (float)(double(bits) * 2.3283064365386963e-10); // / 0x100000000
}

vec2 Sampling::Hammersly2D(unsigned i, unsigned n)
{
	return vec2(float(i) / float(n), radicalInverse_VdC(i));
}

vec2 Sampling::Hammersly2D(unsigned i, unsigned n, const mathfu::vec2 & offset)
{
	vec2 preOffset = Hammersly2D(i, n);
	preOffset += offset;
	return vec2(
		fmodf(preOffset.x, 1.0f),
		fmodf(preOffset.y, 1.0f)
	);
}

vec2 Sampling::OffsetSequence(const vec2& base, const vec2& offset)
{
	float x = fmodf(base.x + offset.x, 1.0f);
	float y = fmodf(base.y + offset.y, 1.0f);
	return vec2(x, y);
}
