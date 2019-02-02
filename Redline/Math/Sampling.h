#pragma once
#include <mathfu/glsl_mappings.h>

namespace Redline
{
	class Sampling
	{
	public:
		static mathfu::vec2 Hammersly2D(unsigned i, unsigned n);
		static mathfu::vec2 Hammersly2D(unsigned i, unsigned n, const mathfu::vec2& offset);
		static mathfu::vec2 OffsetSequence(const mathfu::vec2& base, const mathfu::vec2& offset);
	};
}