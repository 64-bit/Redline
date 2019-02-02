#include "stdafx.h"
#include "TriangleMesh.h"
#include "../objLoader/Source/OBJ_Loader.h"
#include "MeshTriangle.h"
#include "../Math/KDTree/KDTreeNode.h"
#include <tiny_gltf.h>
#include "../FileFormats/GLTFUtilities.h"

using namespace Redline;
using namespace mathfu;
using namespace std;

unsigned int TriangleMesh::GetTriangleCount() const
{
	return (unsigned)TriangleIndicies.size() / 3;
}

TriangleMesh::TriangleMesh(const std::string& name)
{
	Name = name;
	BoundingRadius = 0.0f;
}

TriangleMesh::TriangleMesh(const std::string& name, const objl::Loader& objLoader)
{
	Name = objLoader.LoadedMeshes[0].MeshName;


	//LoadSubmesh(objLoader.LoadedMeshes[1], 0); //TODO:Load materials from OBJ file

	for(const auto& mesh : objLoader.LoadedMeshes)
	{
		LoadSubmesh(mesh,0); //TODO:Load materials from OBJ file
	}

	ComputeTangents();

	printf("Loaded mesh %s with %zu vertices and %u triangles\n", Name.c_str(), Verticies.size(), GetTriangleCount());
}

TriangleMesh::TriangleMesh(const std::string& name, tinygltf::Mesh& model, tinygltf::Model& gltfFile)
{
	Name = name;
	BoundingRadius = 0.0f;

	int primitiveID = 0;
	for(auto& primitive : model.primitives)
	{
		AppendGLTFPrimitive(primitive, gltfFile, primitiveID++);
	}

	ComputeTangents();
	printf("Loaded mesh %s with %zu vertices and %u triangles\n", Name.c_str(), Verticies.size(), GetTriangleCount());
}

std::shared_ptr<TriangleMesh> TriangleMesh::LoadFromOBJFile(const std::string& filename)
{
	objl::Loader objLoader;

	if(!objLoader.LoadFile(filename))
	{
		printf("Could not load obj file:%s\n", filename.c_str());
		return nullptr;
	}

	return make_shared<TriangleMesh>(filename, objLoader);
}

void ComputeTangentBitangent(const MeshVertex& A, const MeshVertex& B, const MeshVertex& C, vec3& tangent, vec3& biTangent)
{
	vec3 pd1 = B.Position - A.Position;
	vec3 pd2 = C.Position - A.Position;


	vec2 d1 = B.UV - A.UV;
	vec2 d2 = C.UV - A.UV;

	vec3 d13 = vec3(d1, 0.0f);
	vec3 d23 = vec3(d2, 0.0f);


	float det = (d1.x * d2.y) - (d1.y * d2.x);
	det = 1.0f / det;

	tangent.x = det * (d2.y * pd1.x + -d1.y * pd2.x);
	tangent.y = det * (d2.y * pd1.y + -d1.y * pd2.y);
	tangent.z = det * (d2.y * pd1.z + -d1.y * pd2.z);

	biTangent.x = det * (-d2.x * pd1.x + d1.x * pd2.x);
	biTangent.y = det * (-d2.x * pd1.y + d1.x * pd2.y);
	biTangent.z = det * (-d2.x * pd1.z + d1.x * pd2.z);
}

void TriangleMesh::ComputeTangents()
{
	//For each triangle, compute tangent and accumulate, we normalise this later.
	for(const auto& triangle : Triangles)
	{
		auto& vertexA = Verticies[triangle.A];
		auto& vertexB = Verticies[triangle.B];
		auto& vertexC = Verticies[triangle.C];

		vec3 tangent, biNormal;
		ComputeTangentBitangent(vertexA, vertexB, vertexC, tangent, biNormal);

		//Add to each vertex
		vertexA.Tangent += tangent;
		vertexB.Tangent += tangent;
		vertexC.Tangent += tangent;

		vertexA.BiNormal += biNormal;
		vertexB.BiNormal += biNormal;
		vertexC.BiNormal += biNormal;
	}

	//For each vertex, normalize tangent and bitangent. also place a copy of the position into the positions only vector
	for (auto& vertex : Verticies)
	{
		VerticiePositionsOnly.push_back(vertex.Position);

		const auto baseT = vertex.Tangent;

		const auto orthorNormTangent = (baseT - vertex.Normal * vec3::DotProduct(vertex.Normal, baseT)).Normalized();

		float handedness = vec3::DotProduct(vec3::CrossProduct(vertex.Normal, baseT), vertex.BiNormal) < 0.0f ? -1.0f : 1.0f;

		vertex.BiNormal = vec3::CrossProduct(vertex.Normal, orthorNormTangent) * handedness;
		vertex.Tangent = orthorNormTangent;
	}
}

void TriangleMesh::AppendGLTFPrimitive(tinygltf::Primitive& primitive, tinygltf::Model& gltfFile,int primitiveID)
{
	auto& indiciesAccessor = gltfFile.accessors[primitive.indices];

	vector<int> indicies;	
	GLTFUtilities::ReadGLTFAccessorToIntVector(indiciesAccessor,gltfFile, indicies);

	//Read attributes
	vector<vec3> positions;
	vector<vec2> uvs;
	vector<vec3> normals;

	//Position
	if(primitive.attributes.find("POSITION") != primitive.attributes.end())
	{
		auto& positionAccessor = gltfFile.accessors[primitive.attributes["POSITION"]];
		GLTFUtilities::ReadGLTFAccessorToVectorVector<vec3>(positionAccessor, gltfFile, positions);
	}
	//UV0
	if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
	{
		auto& uvAccessor = gltfFile.accessors[primitive.attributes["TEXCOORD_0"]];
		GLTFUtilities::ReadGLTFAccessorToVectorVector<vec2>(uvAccessor, gltfFile, uvs);
	}
	//Normals
	if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
	{
		auto& normalAccessor = gltfFile.accessors[primitive.attributes["NORMAL"]];
		GLTFUtilities::ReadGLTFAccessorToVectorVector<vec3>(normalAccessor, gltfFile, normals);
	}

	//Ensure we have all elements
	if(positions.size() != uvs.size()
		|| positions.size() != normals.size())
	{
		return;//ERROR
	}

	int vertexStartPos = (int)Verticies.size();

	//construct vertitices
	for(auto i = 0; i < positions.size();i++)
	{
		MeshVertex vertex;
		vertex.Position = positions[i];
		vertex.UV = uvs[i];
		vertex.Normal = normals[i];
		vertex.BiNormal = vec3(0.0f, 0.0f, 0.0f);
		vertex.Tangent = vec3(0.0f, 0.0f, 0.0f);
		Verticies.push_back(vertex);

		float dist = vertex.Position.Length();
		BoundingRadius = std::max(BoundingRadius, dist);
	}

	//construct triangles
	for(auto i = 0; i < indicies.size()/3;i++)
	{
		MeshTriangle triangle;
		triangle.A = indicies[i * 3 + 0] + vertexStartPos;
		triangle.B = indicies[i * 3 + 1] + vertexStartPos;
		triangle.C = indicies[i * 3 + 2] + vertexStartPos;

		TriangleIndicies.push_back(triangle.A);
		TriangleIndicies.push_back(triangle.B);
		TriangleIndicies.push_back(triangle.C);

		triangle.Material = primitiveID;
		Triangles.push_back(triangle);
	}
}

void Redline::TriangleMesh::LoadSubmesh(const objl::Mesh & mesh, unsigned materialIndex)
{
	unsigned startTriangleIndex = (unsigned)TriangleIndicies.size();
	unsigned currentVertexOffset = (unsigned)Verticies.size();

	for(auto index : mesh.Indices)//Copy Indices from obj loader
	{
		TriangleIndicies.push_back(index + currentVertexOffset);
	}
									//Copy vertices, converting format if necessary in the future
	for (auto i = 0; i < mesh.Vertices.size(); i++)
	{
		auto& srcVertex = mesh.Vertices[i];
		MeshVertex dstVertex;

		dstVertex.Position = vec3(srcVertex.Position.X, srcVertex.Position.Y, srcVertex.Position.Z);
		dstVertex.UV = vec2(srcVertex.TextureCoordinate.X, srcVertex.TextureCoordinate.Y);
		dstVertex.Normal = vec3(srcVertex.Normal.X, srcVertex.Normal.Y, srcVertex.Normal.Z);
		dstVertex.BiNormal = vec3(0.0f, 0.0f, 0.0f);
		dstVertex.Tangent = vec3(0.0f, 0.0f, 0.0f);
		Verticies.push_back(dstVertex);

		float dist = dstVertex.Position.Length();
		BoundingRadius = std::max(BoundingRadius, dist);
	}

	//Create Triangle objects
	for (int i = startTriangleIndex / 3; i < TriangleIndicies.size() / 3; i++)
	{
		MeshTriangle tris;
		tris.Material = materialIndex;

		tris.A = TriangleIndicies[i * 3 + 0];
		tris.B = TriangleIndicies[i * 3 + 1];
		tris.C = TriangleIndicies[i * 3 + 2];

		Triangles.push_back(tris);
	}
}
