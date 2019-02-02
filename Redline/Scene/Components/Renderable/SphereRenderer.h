#pragma once
#include "RenderableSceneComponent.h"
#include <mathfu/glsl_mappings.h>
#include "../../../GraphicalResources/Material.h"

namespace Redline
{
	class SphereRenderer : public RenderableSceneComponent
	{
	public:
		SphereRenderer(SceneObject& owner);
		float Radius;

		bool GetVisableSurfaces(Surface*& outSurface) override;

		void GetDescription() override;

		std::shared_ptr<Material> SurfaceMaterial;
	};
}
