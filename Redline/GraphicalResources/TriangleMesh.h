#pragma once
#include <string>
#include "MeshVertex.h"
#include <vector>
#include "../Math/KDTree/KDTreeNode.h"

namespace objl
{
	struct Mesh;
	class Loader;
}

namespace tinygltf
{
	struct Primitive;
	struct Mesh;
	class Model;
}

namespace Redline
{
	//This class represents a mesh made of triangles. It may contain a number of sub-meshes with different materials.
	//This format is designed to be fast for the raytracer, other file formats may need restructuring to be loaded into this format
	class TriangleMesh
	{

	public:
		std::string Name;

		std::vector<MeshVertex> Verticies;
		std::vector<mathfu::vec3> VerticiePositionsOnly;

		std::vector<unsigned int> TriangleIndicies;
		std::vector<MeshTriangle> Triangles;

		unsigned int GetTriangleCount() const;

		float BoundingRadius;

		//Empty mesh constructor
		TriangleMesh(const std::string& name);

		//Load from OBJ loader object
		TriangleMesh(const std::string& name, const objl::Loader& objLoader);

		//Load from GLTF Mesh object
		TriangleMesh(const std::string& name, tinygltf::Mesh& mesh, tinygltf::Model& gltfFile);

		static std::shared_ptr<TriangleMesh> LoadFromOBJFile(const std::string& filename);

	private:
		void ComputeTangents();

		//Append a GLTF primitive to this mesh
		void AppendGLTFPrimitive(tinygltf::Primitive& primitive, tinygltf::Model& gltfFile, int primitiveID);

		void LoadSubmesh(const objl::Mesh& mesh, unsigned materialIndex);
	};
}
