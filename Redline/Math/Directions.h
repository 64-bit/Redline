#pragma once
#include <mathfu/glsl_mappings.h>

namespace Redline
{
	//Vector defenitions for directions. The render uses a right handed cooridante system with
	//Positive Z being the forward direction
	class Directions
	{
	public:
		static const mathfu::vec3 Forward;
		static const mathfu::vec3 Backward;
		static const mathfu::vec3 Left;
		static const mathfu::vec3 Right;
		static const mathfu::vec3 Up;
		static const mathfu::vec3 Down;
	};
}
