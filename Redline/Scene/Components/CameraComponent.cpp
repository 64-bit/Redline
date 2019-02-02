#include "stdafx.h"
#include "CameraComponent.h"
#include "../SceneObject.h"
#include "../../Math/Math.h"

using namespace Redline;
using namespace mathfu;

CameraComponent::CameraComponent(SceneObject& parent)
	:SceneObjectComponent(parent)
{
	YAxisFieldofViewRadians = 90.0f;
	AspectRatio = 4.0f / 3.0f;
}

mat4 CameraComponent::GetViewMatrix()
{
	auto worldMatrix = Object.Transform.GetWorldTransformMatrix();
	return worldMatrix.Inverse();
}