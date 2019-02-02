#include "stdafx.h"
#include "MeshTriangle.h"
#include "../Math/BoundingBox.h"

using namespace Redline;

BoundingBox BVHTriangle::GetBounds()
{
	BoundingBox bounds;
	bounds.EnlargeByPoint(A.Position);
	bounds.EnlargeByPoint(B.Position);
	bounds.EnlargeByPoint(C.Position);
	return bounds;
}