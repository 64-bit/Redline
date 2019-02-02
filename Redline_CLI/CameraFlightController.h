#pragma once
#include "Scene/Components/CameraComponent.h"
#include <mathfu/glsl_mappings.h>
#include "RenderApplication.h"

namespace Redline
{
	class CameraFlightController
	{
	public:
		CameraFlightController(RenderApplication& renderApplication, std::shared_ptr<CameraComponent>& camera);

		bool OnSDLUpdate(float deltaT);
	private:

		RenderApplication& _renderApplication;

		std::shared_ptr<CameraComponent> _camera;
		int _lastMouseX;
		int _lastMouseY;
		mathfu::vec3 _cameraPitchYawRoll;

		float _timeSinceLastRenderUpdate;
		bool _hasMovedSinceLastUpdate;

		bool AttemptTranslations(float deltaT);

		bool AttemptRotation(float deltaT);
	};
}
