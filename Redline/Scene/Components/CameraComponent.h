#pragma once
#include "../SceneObjectComponent.h"
#include "../SceneForwardDeclarations.h"
#include <mathfu/glsl_mappings.h>

namespace Redline
{
	class CameraComponent : public SceneObjectComponent
	{
	public:
		float YAxisFieldofViewRadians;
		float AspectRatio;//Note, this is overriden by the requested render size in most cases

		CameraComponent(SceneObject& parent);

		mathfu::mat4 GetViewMatrix();
	private:
	};
}
