#pragma once
#include <vector>
#include "Raytracer/Lights/DirectionalLight.h"
#include "Raytracer/Lights/PointLight.h"
#include "Raytracer/Lights/SpotLight.h"

namespace Redline
{

	/*	Structure that contains all lighting data about a scene, needed to render a single frame
	 *	This includes the following information
	 *		- All point lights
	 *		- All spot lights
	 *		- All directional lights
	 *		- All Area/Mesh/Other odd light types we add in the future
	 *		- Ambient light
	 *		- World Environment cubemap
	 */
	struct SceneLightingData
	{
		std::vector<Light*> SceneLights;

		std::vector<DirectionalLight> DirectionalLights;
		std::vector<PointLight> PointLights;
		std::vector<SpotLight> SpotLights;

		mathfu::vec3 AmbientLight;
		mathfu::vec3 BackgroundColor;
		float EnvironmentPower;
		std::shared_ptr<CubemapTexture> EnvironmentCubemap;

	};
}
