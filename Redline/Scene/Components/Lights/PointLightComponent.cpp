#include "stdafx.h"
#include "PointLightComponent.h"

Redline::PointLightComponent::PointLightComponent(SceneObject & owner)
	: SceneObjectComponent(owner)
{
	LightSettings.Color = mathfu::vec3(1.0f, 1.0f, 1.0f);
	LightSettings.Power = 1.0f;
	LightSettings.LightRadius = 1.0f; //Technicly not physical, but.
	LightSettings.PhysicalRadius = 0.1f;
}
