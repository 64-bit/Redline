#pragma once

#include "SceneForwardDeclarations.h"

namespace Redline
{
	//This is a base class for all components that are attached to a sceneObject, including the transform
	class SceneObjectComponent
	{
		
	public:
		SceneObject& Object;
		Scene& HomeScene;

	protected:
		SceneObjectComponent(SceneObject& object);

	public:

		virtual void GetDescription();
	};
}
