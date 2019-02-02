#include "stdafx.h"
#include "BVH.h"
#include "../../Renderer/Raytracer/Surfaces/Surface.h"

using namespace Redline;
using namespace std;
using namespace mathfu;




template<typename T>
float BVHNode<T>::GetVolume() const
{
	vec3 size = Bounds.Max - Bounds.Min;
	return size.x * size.y * size.z;
}

template<typename T>
float BVHNode<T>::GetVolumeIfEnlargedByOther(const BVHNode<T>* otherNode)
{
	vec3 combinedMin = vec3::Min(Bounds.Min, otherNode->Bounds.Min);
	vec3 combinedMax = vec3::Max(Bounds.Max, otherNode->Bounds.Max);

	vec3 combinedSize = combinedMax - combinedMin;
	return combinedSize.x * combinedSize.y * combinedSize.z;
}

