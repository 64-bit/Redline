#pragma once

#include <mathfu/glsl_mappings.h>
#include "../../SceneObjectComponent.h"
#include "../../../Renderer/Raytracer/Lights/DirectionalLight.h"
#include "../../../Renderer/Raytracer/Lights/PointLight.h"

namespace Redline
{
	class SceneObject;

	class PointLightComponent : public SceneObjectComponent
	{
	public:
		PointLightComponent(SceneObject& owner);
		PointLight LightSettings;
	};
}
