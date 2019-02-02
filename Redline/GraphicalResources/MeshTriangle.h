#pragma once
#include "MeshVertex.h"

namespace Redline
{
	class BoundingBox;

	struct MeshTriangle
	{
		unsigned A;
		unsigned B;
		unsigned C;
		unsigned Material;
	};

	struct BVHTriangle
	{
		MeshVertex A;
		MeshVertex B;
		MeshVertex C;
		unsigned Material;

		BoundingBox GetBounds();
	};
}
