#include "stdafx.h"
#include "KDTreeNode.h"

using namespace Redline;
using namespace mathfu;
using namespace std;

KDTreeNode::KDTreeNode()
{
	Left = nullptr;
	Right = nullptr;
}

void KDTreeNode::InsertPoints(const std::vector<MeshTriangle>& triangles, const std::vector<MeshVertex>& verticies, int parentSplitPlane)
{
	if(triangles.size() < MINIMUM_TRIANGLES_TO_SPLIT)
	{
		//place all triangles into this node
		Triangles = triangles;
		LocalTriangleBounds.EnlargeByTriangles(Triangles, verticies);
		return;
	}

	//Else, we need to determin a split plane
	SplitPlane = parentSplitPlane + 1;
	if(SplitPlane > 2)
	{
		SplitPlane = 0;
	}

	//And then average the location of all triangles, we can just average all vertice positions
	float axisAverage = 0.0f;

	for(const auto& triangle : triangles)
	{
		axisAverage += verticies[triangle.A].Position[SplitPlane];
		axisAverage += verticies[triangle.B].Position[SplitPlane];
		axisAverage += verticies[triangle.C].Position[SplitPlane];
	}
	SplitPoint = axisAverage / (triangles.size() * 3);

	//Now we need to sort the triangles to be left,right or straddleing this split point
	vector<MeshTriangle> leftTriangles;
	vector<MeshTriangle> rightTriangles;

	for(const auto& triangle : triangles)
	{
		float a = verticies[triangle.A].Position[SplitPlane];
		float b = verticies[triangle.B].Position[SplitPlane];
		float c = verticies[triangle.C].Position[SplitPlane];

		if(a > SplitPoint
			&& b > SplitPoint
			&& c > SplitPoint)
		{
			rightTriangles.push_back(triangle);
		}
		else if(a < SplitPoint
			&& b < SplitPoint
			&& c < SplitPoint)
		{
			leftTriangles.push_back(triangle);
		}
		else
		{
			Triangles.push_back(triangle);//If we are not all over or under, we straddle the border and belong to this node
		}
	}

	//Build local bounds
	LocalTriangleBounds.EnlargeByTriangles(Triangles, verticies);

	//Build child lists if anything ended up in the left or right buckets
	if(leftTriangles.size() > 0)
	{
		Left = new KDTreeNode();
		Left->InsertPoints(leftTriangles, verticies, SplitPlane);
	}

	if (rightTriangles.size() > 0)
	{
		Right = new KDTreeNode();
		Right->InsertPoints(rightTriangles, verticies, SplitPlane);
	}

}
