#pragma once

#include <mathfu/glsl_mappings.h>
#include "../../SceneObjectComponent.h"
#include "../../../Renderer/Raytracer/Lights/DirectionalLight.h"

namespace Redline
{
	class SceneObject;

	class DirectionalLightComponent : public SceneObjectComponent
	{
	public:
		DirectionalLightComponent(SceneObject& owner);
		DirectionalLight LightSettings;
	};
}
