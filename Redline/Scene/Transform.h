#pragma once
#include <mathfu/glsl_mappings.h>
#include "SceneForwardDeclarations.h"
#include "SceneObjectComponent.h"
#include <vector>

namespace Redline
{
	//Represents the transform of a given scene object.
	//The transform values stored in this class are the local transform of that given node.
	//To determin the world space position, this transform must be combined with the worldspace transform of this object's parents
	class Transform : public SceneObjectComponent
	{

	private:
		//Components of local transform
		mathfu::vec3 _localPosition;
		mathfu::quat _localRotation;
		mathfu::vec3 _localScale;

		Transform* Parent;
		std::vector<Transform*> Children;
	public:
		Transform(SceneObject& owningObject);

		//Get a matrix that transforms a point in object space to world space.
		mathfu::mat4 GetWorldTransformMatrix() const;

		//Get a matrix that transforms a point in object space to the space of this object's parent
		mathfu::mat4 GetLocalTransformMatrix() const;

		void SetParent(Transform* newParent);

		const std::vector<Transform*>& GetChildren() const;


		//Returns the position of this transform relative to it's parent
		mathfu::vec3 GetLocalPosition() const;

		//Returns the position of this transform relative to the origin (0,0,0)
		mathfu::vec3 GetPosition() const;

		//Set the position of this transform relative to it's parent
		void SetLocalPosition(const mathfu::vec3& position);

		//Set the position of this transform relative to the origin (0,0,0)
		void SetPosition(const mathfu::vec3& position);

		//Returns the rotation of this transform relative to it's parent
		mathfu::quat GetLocalRotation() const;

		//Returns the rotation of this transform relative to the origin (0,0,0)
		mathfu::quat GetRotation() const;

		//Set the rotation of this transform relative to it's parent
		void SetLocalRotation(const mathfu::quat& rotation);

		//Set the rotation of this transform relative to the origin (0,0,0)
		void SetRotation(const mathfu::quat& rotation);

		//Returns the scale of this transform relative to it's parent
		mathfu::vec3 GetLocalScale() const;

		//Returns the scale of this transform relative to a object scaled (1,1,1,)
		mathfu::vec3 GetScale() const;

		//Set the scale of this transform relative to it's parent
		void SetLocalScale(const mathfu::vec3& scale);

		//Set the scale of this transform relative to a object scaled (1,1,1,)
		void SetScale(const mathfu::vec3& scale);

		mathfu::vec3 Forwards() const;

		mathfu::vec3 Right() const;

		mathfu::vec3 Up() const;

	};

}