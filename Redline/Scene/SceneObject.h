#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Transform.h"
#include "SceneForwardDeclarations.h"

namespace Redline
{
	/*
	 * Represents a object in the scenegraph
	 * A SceneObject has a transform that represents it's location relative to it's parent.
	 * A SceneObject has 0 or more children
	 * A SceneObject has 0 or more components, that the render uses to draw the scene
	 */
	class SceneObject
	{
	public:
		//The name of the SceneObject, I find it useful if everything has a name when trying to debug issues.
		std::string Name;

		//The scene this SceneObject belongs to.
		Scene& HomeScene;

		//Transform
		Transform Transform;

		//Components
		std::vector < std::shared_ptr<SceneObjectComponent>> Components;

		//Constructor for SceneObject, this should be called by the method Scene::CreateSceneObject
		SceneObject(Scene& homeScene, const std::string& objectName);

		//Add a component of the specified type to this scene object, and return a shared pointer to the new component
		template <class T, typename... Args>
		std::shared_ptr<T> AddComponent(Args... args);

		template <class T>
		std::shared_ptr<T> TryGetComponent();

	};

	template <class T, typename... Args>
	std::shared_ptr<T> SceneObject::AddComponent(Args... args)
	{
		//Create component passing args though
		auto newComponent = std::make_shared<T>(*this,args...);
		//Add to component list
		Components.push_back(newComponent);
		return newComponent;
	}

	template <class T>
	std::shared_ptr<T> SceneObject::TryGetComponent()
	{
		for(auto& component : Components)
		{
			auto castAttempt = std::dynamic_pointer_cast<T>(component);
			if(castAttempt != nullptr)
			{
				return castAttempt;
			}
		}

		return nullptr;
	}
}
