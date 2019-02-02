#include "stdafx.h"
#include "RenderableSceneComponent.h"

using namespace Redline;

RenderableSceneComponent::RenderableSceneComponent(SceneObject & owner)
	: SceneObjectComponent(owner)
{
	IsVisable = true;
}
