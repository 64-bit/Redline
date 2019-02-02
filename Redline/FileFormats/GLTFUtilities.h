#pragma once
#include <vector>
#include <tiny_gltf.h>
#include "mathfu/glsl_mappings.h"

namespace Redline
{
	class GLTFUtilities
	{
	public:
		static void ReadGLTFAccessorToIntVector(
			tinygltf::Accessor& accessor,
			tinygltf::Model& gltfFile,
			std::vector<int>& resultsOut);

		static void ReadGLTFAccessorToFloatVector(
			tinygltf::Accessor& accessor,
			tinygltf::Model& gltfFile,
			std::vector<float>& resultsOut);

		template<class T>
		static void ReadGLTFAccessorToVectorVector(
			tinygltf::Accessor& accessor,
			tinygltf::Model& gltfFile,
			std::vector<T>& resultsOut);
	};

	template<class T>
	inline void GLTFUtilities::ReadGLTFAccessorToVectorVector(tinygltf::Accessor & accessor, tinygltf::Model & gltfFile, std::vector<T>& resultsOut)
	{
		auto& bufferView = gltfFile.bufferViews[accessor.bufferView];
		auto& buffer = gltfFile.buffers[bufferView.buffer];
		void* bufferData = static_cast<void*>(buffer.data.data());

		if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
		{
			return;
		}

		size_t viewOffset = bufferView.byteOffset;
		size_t accessorOffset = accessor.byteOffset;
		//How does this work with 
		size_t byteStride = bufferView.byteStride;

		int components = 0;

		switch (accessor.type)
		{
		case TINYGLTF_TYPE_VEC2:
			components = 2;
			break;
		case TINYGLTF_TYPE_VEC3:
			components = 3;
			break;
		case TINYGLTF_TYPE_VEC4:
			components = 4;
			break;
		default:
			return;//TODO:Handle invalid values
		}

		if (components != T::d)
		{
			return;//mismatch of vector and accessor
		}

		if (byteStride == 0)
		{
			byteStride = sizeof(float) * components;
		}

		unsigned char* dataPtr = buffer.data.data();

		dataPtr += viewOffset;
		dataPtr += accessorOffset;

		for (size_t i = 0; i < accessor.count; i++)
		{
			T vector;

			for (int c = 0; c < components; c++) 
			{
				float val = *reinterpret_cast<float*>(dataPtr + c*4);
				vector[c] = val;
			}

			resultsOut.push_back(vector);
			dataPtr += byteStride;
		}
	}
}
