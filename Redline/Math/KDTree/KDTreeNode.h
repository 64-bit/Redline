#pragma once
#include <vector>
#include "../../GraphicalResources/MeshVertex.h"
#include "../../GraphicalResources/MeshTriangle.h"
#include "../BoundingBox.h"

namespace Redline
{

	const int MINIMUM_TRIANGLES_TO_SPLIT = 16;

	class KDTreeNode
	{
	public:

		int SplitPlane;

		float SplitPoint;

		KDTreeNode* Left;//Negative
		KDTreeNode* Right;//Postive

		std::vector<MeshTriangle> Triangles;

		KDTreeNode();

		void InsertPoints(const std::vector<MeshTriangle>& triangles, const std::vector<MeshVertex>& verticies, int parentSplitPlane);

		//This represents the bounds of the triangles local to this node,
		// IRRESPECTIVE of what the child nodes contain
		BoundingBox LocalTriangleBounds;
	};
}
