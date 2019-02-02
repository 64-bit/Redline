#pragma once
#include <mathfu/glsl_mappings.h>


namespace Redline
{
	struct Ray
	{
	public:
		mathfu::vec3 Origin;
		mathfu::vec3 Direction;
	};
}
