#pragma once
#include "../../SceneObjectComponent.h"

namespace Redline
{
	class Surface;

	//This is a base abstract class for anything that the raytracer can render.
	class RenderableSceneComponent : public SceneObjectComponent
	{
	public:
		bool IsVisable;

		//Get visable geometry surfaces. this is used by the renderer to build it's internal accleration structure.
		//Returns true if any geometry was returned
		virtual bool GetVisableSurfaces(Surface*& outSurface) = 0;

	protected:
		RenderableSceneComponent(SceneObject& owner);
	};
}
