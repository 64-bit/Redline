#pragma once

#include "../../GraphicalResources/TriangleMesh.h"

namespace Redline
{
	
	/*
	 * A KD tree used for spatial search of triangles in a mesh
	 */
	class KDTree
	{
		
	public:

		void BuildFromMesh(std::shared_ptr<TriangleMesh> mesh);

	private:




	};


}
