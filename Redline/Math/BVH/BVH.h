#pragma once
#include "../BoundingBox.h"
#include <algorithm>

namespace Redline
{
	class Surface;

	//T must implement BoundingBox GetBounds() method. it can be either a pointer(used for surfaces) or just a type (used for triangles)
	template <typename T>
	class BVHNode
	{
	public:
		BoundingBox Bounds;
		std::vector<BVHNode<T>*>ChildNodes;
		std::vector<T> ContentsVector;

		BVHNode();
		~BVHNode();

		static BVHNode<T>* BuildBVH(const std::vector<T>& contents, size_t maxPageSize);

		float GetVolume() const;

		float GetVolumeIfEnlargedByOther(const BVHNode<T>* otherNode);

		BoundingBox GetBounds()
		{
			return Bounds;
		}

		void AddChild(BVHNode<T>* child);

		template <class T2, std::enable_if_t<std::is_pointer<T2>::value,int> = 0>
		void AddElement(T2& element)
		{
			ContentsVector.push_back(element);
			Bounds.EnlargeByBounds(element->GetBounds());
		}

		template <class T2, std::enable_if_t<!std::is_pointer<T2>::value,int> = 0>
		void AddElement(T2& element)
		{
			ContentsVector.push_back(element);
			Bounds.EnlargeByBounds(element.GetBounds());
		}

	private:

		template <class T2, std::enable_if_t<std::is_pointer<T2>::value, int> = 0>
		static void SortElements(std::vector<T2>& elements, int currentDimension)
		{
			std::sort(elements.begin(), elements.end(),
				[currentDimension](T2 a, T2 b) -> bool
			{
				mathfu::vec3 centerA = a->GetBounds().GetCenter();
				mathfu::vec3 centerB = b->GetBounds().GetCenter();

				return centerA[currentDimension] > centerB[currentDimension];
			});
		}

		template <class T2, std::enable_if_t<!std::is_pointer<T2>::value, int> = 0>
		static void SortElements(std::vector<T2>& elements, int currentDimension)
		{
			std::sort(elements.begin(), elements.end(),
				[currentDimension](T2 a, T2 b) -> bool
			{
				mathfu::vec3 centerA = a.GetBounds().GetCenter();
				mathfu::vec3 centerB = b.GetBounds().GetCenter();

				return centerA[currentDimension] > centerB[currentDimension];
			});
		}

		static std::vector<BVHNode<T>*> CreateLeafNodes(const std::vector<T>& contents, size_t maxPageSize);

		std::vector<BVHNode<T>*> static SortTileRecursive(size_t maxPageSize, std::vector<T>& elements, int currentDimension, int splits);

		std::vector<BVHNode<T>*> static SortTileRecursiveNodes(size_t maxPageSize, std::vector<BVHNode<T>*>& nodes, int currentDimension, int splits);

		static int EstimateTileSplits(size_t nodeCount, size_t elementsPerPage);
	};

	template<typename T>
	BVHNode<T>::BVHNode()
	{
	}

	template<typename T>
	BVHNode<T>::~BVHNode()
	{
		for (auto child : ChildNodes)
		{
			delete child;
		}
		ChildNodes.clear();
	}

	//New method to build BVH
	template<typename T>
	BVHNode<T>* BVHNode<T>::BuildBVH(const std::vector<T>& contents, size_t maxPageSize)
	{
		//Create leaf nodes
		auto leafNodes = CreateLeafNodes(contents, maxPageSize);
		if(contents.size() > 20)
		{
			printf("Created %d leaf nodes from %d contents\n", (int)leafNodes.size(), (int)contents.size());
		}
		while(leafNodes.size() > 1)
		{
			//int splits = EstimateTileSplits(leafNodes.size(), maxPageSize);
			int splits = 2;// This gives us 8 children
			leafNodes = SortTileRecursiveNodes(8, leafNodes, 0, splits);
		}
		return leafNodes[0];
	}

	template <typename T>
	std::vector<BVHNode<T>*> BVHNode<T>::CreateLeafNodes(const std::vector<T>& contents, size_t maxPageSize)
	{
		std::vector<T> copy = contents;//Deep copy as to not mess with mesh source
		int splits = EstimateTileSplits(copy.size(), maxPageSize);
		return SortTileRecursive(maxPageSize, copy, 0, splits);
	}

	template<typename T>
	std::vector<BVHNode<T>*> BVHNode<T>::SortTileRecursive(size_t maxPageSize, std::vector<T>& elements, int currentDimension, int splits)
	{
		std::vector<BVHNode<T>*> results;

		if (elements.size() <= maxPageSize)
		{
			BVHNode<T>* bundle = new BVHNode<T>();
			for (auto& element : elements)
			{
				bundle->AddElement(element);
			}
			results.push_back(bundle);
			return results;
		}

		//Sort all elements along the current dimension, sort by bounding box centerpoint
		SortElements(elements, currentDimension);

		int elementsPerSplit = ((int)elements.size() / splits) + 1;

		for (int split = 0; split < splits; split++)
		{
			//For each split, bundle nodes into new vector
			std::vector<T> elementsThisSplit;		
			for (int element = 0; element < elementsPerSplit; element++)
			{
				int elementAddress = element + split * elementsPerSplit;

				if (elementAddress >= elements.size())
				{
					break;//this handles the case of the last bucket that will not be full
				}
				elementsThisSplit.push_back(elements[elementAddress]);
			}

			//If this is the z-axis, bundle up nodes into one and return a vector
			if (currentDimension == 2)
			{
				BVHNode<T>* bundle = new BVHNode<T>();
				for (auto& element : elementsThisSplit)
				{
					bundle->AddElement(element);
				}
				if (bundle->ContentsVector.size() > 0)
				{
					results.push_back(bundle);
				}
			}
			//Else, bundle up nodes and recurse down.
			else
			{
				auto nextLevelResults = SortTileRecursive(maxPageSize, elementsThisSplit, currentDimension + 1, splits);
				results.insert(results.end(), nextLevelResults.begin(), nextLevelResults.end());
			}
		}

		return results;
	}

	template<typename T>
	std::vector<BVHNode<T>*> BVHNode<T>::SortTileRecursiveNodes(size_t maxPageSize, std::vector<BVHNode<T>*>& elements, int currentDimension, int splits)
	{
		std::vector<BVHNode<T>*> results;

		if(elements.size() <= maxPageSize)
		{
			BVHNode<T>* bundle = new BVHNode<T>();
			for (auto& element : elements)
			{
				bundle->AddChild(element);
			}
			results.push_back(bundle);
			return results;
		}

		//Sort all elements along the current dimension, sort by bounding box centerpoint
		SortElements(elements, currentDimension);

		int elementsPerSplit = ((int)elements.size() / splits) + 1;

		for (int split = 0; split < splits; split++)
		{
			//For each split, bundle nodes into new vector
			std::vector<BVHNode<T>*> elementsThisSplit;
			for (int element = 0; element < elementsPerSplit; element++)
			{
				int elementAddress = element + split * elementsPerSplit;

				if (elementAddress >= elements.size())
				{
					break;//this handles the case of the last bucket that will not be full
				}
				elementsThisSplit.push_back(elements[elementAddress]);
			}

			//If this is the z-axis, bundle up nodes into one and return a vector
			if (currentDimension == 2)
			{
				BVHNode<T>* bundle = new BVHNode<T>();
				for (auto& element : elementsThisSplit)
				{
					bundle->AddChild(element);
				}
				if (bundle->ChildNodes.size() > 0)
				{
					results.push_back(bundle);
				}
			}
			//Else, bundle up nodes and recurse down.
			else
			{
				auto nextLevelResults = SortTileRecursiveNodes(maxPageSize, elementsThisSplit, currentDimension + 1, splits);
				results.insert(results.end(), nextLevelResults.begin(), nextLevelResults.end());
			}
		}
		return results;
	}

	template<typename T>
	void BVHNode<T>::AddChild(BVHNode<T>* child)
	{
		Bounds.EnlargeByBounds(child->Bounds);
		ChildNodes.push_back(child);
	}

	template<typename T>
	int BVHNode<T>::EstimateTileSplits(size_t nodeCount, size_t elementsPerPage)
	{
		size_t estimatedLeafs = nodeCount / elementsPerPage;
		float floatSplits = powf((float)estimatedLeafs, (1.0f / 3.0f));
		int intSplits = (int)ceilf(floatSplits);
		return std::max(intSplits, 1);
	}
}