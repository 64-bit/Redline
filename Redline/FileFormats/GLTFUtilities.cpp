#include "stdafx.h"
#include "GLTFUtilities.h"

using namespace Redline;
using namespace tinygltf;
using namespace std;

void GLTFUtilities::ReadGLTFAccessorToIntVector(Accessor& accessor, Model& gltfFile, vector<int>& resultsOut)
{
	auto& bufferView = gltfFile.bufferViews[accessor.bufferView];
	auto& buffer = gltfFile.buffers[bufferView.buffer];
	void* bufferData = static_cast<void*>(buffer.data.data());

	if ((accessor.componentType != TINYGLTF_COMPONENT_TYPE_INT
		&& accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
		|| accessor.type != TINYGLTF_TYPE_SCALAR)
	{
		//Invalid for index buffers
		return;
	}

	size_t viewOffset = bufferView.byteOffset;
	size_t accessorOffset = accessor.byteOffset;
	//How does this work with 
	size_t byteStride = bufferView.byteStride;
	if(byteStride == 0)
	{
		byteStride = sizeof(int);
	}

	unsigned char* dataPtr = buffer.data.data();

	dataPtr += viewOffset;
	dataPtr += accessorOffset;

	if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT)
	{
		for (size_t i = 0; i < accessor.count; i++)
		{
			int val = *reinterpret_cast<int*>(dataPtr);
			resultsOut.push_back(val);
			dataPtr += byteStride;
		}
	}
	else
	{
		for (size_t i = 0; i < accessor.count; i++)
		{
			int val = (int)*reinterpret_cast<unsigned*>(dataPtr);
			resultsOut.push_back(val);
			dataPtr += byteStride;
		}
	}

}

void GLTFUtilities::ReadGLTFAccessorToFloatVector(Accessor& accessor, Model& gltfFile, std::vector<float>& resultsOut)
{
	auto& bufferView = gltfFile.bufferViews[accessor.bufferView];
	auto& buffer = gltfFile.buffers[bufferView.buffer];
	void* bufferData = static_cast<void*>(buffer.data.data());

	if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT
		|| accessor.type != TINYGLTF_TYPE_SCALAR)
	{
		//Invalid for index buffers
		return;
	}

	size_t viewOffset = bufferView.byteOffset;
	size_t accessorOffset = accessor.byteOffset;
	//How does this work with 
	size_t byteStride = bufferView.byteStride;
	if (byteStride == 0)
	{
		byteStride = sizeof(float);
	}

	unsigned char* dataPtr = buffer.data.data();

	dataPtr += viewOffset;
	dataPtr += accessorOffset;

	for (size_t i = 0; i < accessor.count; i++)
	{
		float val = *reinterpret_cast<float*>(dataPtr);
		resultsOut.push_back(val);
		dataPtr += byteStride;
	}
}
