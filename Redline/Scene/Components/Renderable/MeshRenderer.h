#pragma once
#include "RenderableSceneComponent.h"
#include <mathfu/glsl_mappings.h>
#include "../../../GraphicalResources/Material.h"
#include "../../../GraphicalResources/TriangleMesh.h"

namespace Redline
{
	class MeshRenderer : public RenderableSceneComponent
	{
	public:
		MeshRenderer(SceneObject& owner);

		std::shared_ptr<TriangleMesh> Mesh;
		std::vector<std::shared_ptr<Material>> SurfaceMaterials;

		bool GetVisableSurfaces(Surface*& outSurface) override;

		void GetDescription() override;
	};
}
