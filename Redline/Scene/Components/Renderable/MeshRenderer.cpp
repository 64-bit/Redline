#include "stdafx.h"
#include "MeshRenderer.h"
#include "../../SceneObject.h"
#include "../../../Renderer/Raytracer/Surfaces/TriangleMeshSurface.h"

using namespace Redline;

Redline::MeshRenderer::MeshRenderer(SceneObject & owner)
	: RenderableSceneComponent(owner)
{
	Mesh = nullptr;
	/*SurfaceMaterial = std::make_shared<Material>();
	SurfaceMaterial->Albedo = mathfu::vec3(0.8f, 0.8f, 0.8f);
	SurfaceMaterial->Metalness = 0.0f;*/
}

bool Redline::MeshRenderer::GetVisableSurfaces(Surface*& outSurface)
{
	auto pos = Object.Transform.GetWorldTransformMatrix().TranslationVector3D();

	auto meshSurface = new TriangleMeshSurface(Object.Transform, Mesh, SurfaceMaterials);
	outSurface = meshSurface;

	return true;
}

void Redline::MeshRenderer::GetDescription()
{

}