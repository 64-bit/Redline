#include "stdafx.h"
#include "SceneObjectComponent.h"
#include "SceneObject.h"

using namespace Redline;

Redline::SceneObjectComponent::SceneObjectComponent(SceneObject& object)
	: Object(object),
		HomeScene(object.HomeScene)
{

}

void SceneObjectComponent::GetDescription()
{
}
