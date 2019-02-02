#include "stdafx.h"
#include "DirectionalLightComponent.h"

Redline::DirectionalLightComponent::DirectionalLightComponent(SceneObject & owner)
	:SceneObjectComponent(owner)
{
	LightSettings.Color = mathfu::vec3(0.8f, 0.8f, 0.8f);
	LightSettings.Power = 0.5f;
	LightSettings.Direction = mathfu::vec3(0.5f, -0.2f, 0.5f).Normalized();
	//LightSettings.Direction = mathfu::vec3(0.5f, -0.5f, 0.5f).Normalized();
	LightSettings.PhysicalRadius = 695700.0f; //The default value is the raidus of the real sun in KM
	LightSettings.PhysicalDistance = 1.495E8; //The default value is the distance of the real sun in KM
}