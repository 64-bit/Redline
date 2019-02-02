#pragma once
#include "Scene/SceneObjectComponent.h"
#include "Renderer/Raytracer/Lights/SpotLight.h"

namespace Redline
{
	class SpotLightComponent : public SceneObjectComponent
	{
	public:
		SpotLightComponent(SceneObject& owner);
		SpotLight LightSettings;
	};
}
