#include "stdafx.h"
#include "SphereRenderer.h"
#include "../../../Renderer/Raytracer/Surfaces/SphereSurface.h"
#include "../../SceneObject.h"

Redline::SphereRenderer::SphereRenderer(SceneObject& owner)
	: RenderableSceneComponent(owner)
{
	Radius = 1.0f;
	SurfaceMaterial = std::make_shared<Material>();
	SurfaceMaterial->Albedo = mathfu::vec3(0.8f, 0.8f, 0.8f);
}

bool Redline::SphereRenderer::GetVisableSurfaces(Surface*& outSurface)
{
	auto pos = Object.Transform.GetWorldTransformMatrix().TranslationVector3D();

	outSurface = new SphereSurface(pos,Radius,SurfaceMaterial);
	return true;
}

void Redline::SphereRenderer::GetDescription()
{

}
