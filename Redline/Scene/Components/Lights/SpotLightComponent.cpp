#include "stdafx.h"
#include "SpotLightComponent.h"

using namespace Redline;
using namespace mathfu;

SpotLightComponent::SpotLightComponent(SceneObject & owner)
	:SceneObjectComponent(owner)
{
	LightSettings.Color = vec3(1.0f, 1.0f, 1.0f);
	LightSettings.Power = 1.0f;
	LightSettings.LightRadius = 1.0f; //Technicly not physical, but.
	LightSettings.PhysicalRadius = 0.1f;

	LightSettings.InnerAngleCos = 0.2f;
	LightSettings.OuterAngleCos = 0.3f;
}
