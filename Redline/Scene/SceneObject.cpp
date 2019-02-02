#include "stdafx.h"
#include "SceneObject.h"

using namespace Redline;

SceneObject::SceneObject(Scene& homeScene, const std::string& objectName)
	: HomeScene(homeScene) ,
		Transform(*this)//Assign transform reference
{
	Name = objectName;
}

