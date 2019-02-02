#include "stdafx.h"
#include "CameraFlightController.h"
#include <Scene/SceneObject.h>

#include "SDL.h"
#undef main

using namespace Redline;
using namespace std;
using namespace mathfu;

const float CAMERA_SPEED = 2.0f;
const float MOUSE_SENSITIVITY = 500.0f;

const float DELAY_BETWEEN_UPDATES = 0.05f;

CameraFlightController::CameraFlightController(RenderApplication & renderApplication,
	std::shared_ptr<CameraComponent>& camera)
	: _renderApplication(renderApplication)
{
	_camera = camera;
	_lastMouseX = 0;
	_lastMouseY = 0;
	_cameraPitchYawRoll = ExtractPitchYawRoll(camera->Object.Transform.GetRotation());
	_timeSinceLastRenderUpdate = 0.0f;
	_hasMovedSinceLastUpdate = false;
}

bool CameraFlightController::OnSDLUpdate(float deltaT)
{
	const bool didTranslate = AttemptTranslations(deltaT);
	const bool didRotate = AttemptRotation(deltaT);

	_hasMovedSinceLastUpdate =
		_hasMovedSinceLastUpdate ||
		didTranslate || didRotate;

	_timeSinceLastRenderUpdate += deltaT;

	if(_timeSinceLastRenderUpdate >= DELAY_BETWEEN_UPDATES
		&& _hasMovedSinceLastUpdate)
	{
		_hasMovedSinceLastUpdate = false;
		_timeSinceLastRenderUpdate = 0.0f;
		return true;
	}

	return false;
}

bool CameraFlightController::AttemptTranslations(float deltaT)
{
	const Uint8 *state = SDL_GetKeyboardState(nullptr);
	vec3 move(0.0f, 0.0f, 0.0f);

	auto& cameraObjectTransform = _camera->Object.Transform;

	if (state[SDL_SCANCODE_W])
	{
		move += cameraObjectTransform.Forwards();
	}
	if (state[SDL_SCANCODE_S])
	{
		move -= cameraObjectTransform.Forwards();
	}
	if (state[SDL_SCANCODE_A])
	{
		move -= cameraObjectTransform.Right();
	}
	if (state[SDL_SCANCODE_D])
	{
		move += cameraObjectTransform.Right();
	}
	if (state[SDL_SCANCODE_Q])
	{
		move += vec3(0.0f, -1.0f, 0.0f);
	}
	if (state[SDL_SCANCODE_E])
	{
		move += vec3(0.0f, 1.0f, 0.0f);
	}

	if(move.Length() > 0.0f) //If we have any sort of movement input, preform the movement and return true
	{
		move = move * deltaT * CAMERA_SPEED;

		auto cameraPos = cameraObjectTransform.GetPosition();
		cameraPos += move;
		cameraObjectTransform.SetPosition(cameraPos);
		return true;
	}

	return false;
}

bool CameraFlightController::AttemptRotation(float deltaT)
{
	int currentMouseX, currentMouseY;
	const auto buttonState = SDL_GetMouseState(&currentMouseX, &currentMouseY);

	bool didCameraMove = false;

	if ((buttonState & 0x1) > 0)
	{
		if (currentMouseX != _lastMouseX ||
			currentMouseY != _lastMouseY)
		{
			didCameraMove = true;

			int deltaX = currentMouseX - _lastMouseX;
			int deltaY = currentMouseY - _lastMouseY;

			float pitchMove = (float)deltaY / MOUSE_SENSITIVITY;
			float yawMove = (float)deltaX / MOUSE_SENSITIVITY;

			_cameraPitchYawRoll[0] -= pitchMove;
			_cameraPitchYawRoll[1] -= yawMove;

			const auto newPitch = quat::FromEulerAngles(_cameraPitchYawRoll[0], 0.0f, 0.0f);
			const auto newYaw = quat::FromEulerAngles(0.0f, _cameraPitchYawRoll[1], 0.0f);

			//Rotate camera
			auto currentCameraRotation = _camera->Object.Transform.GetRotation();

			auto newCameraRotation = newYaw * newPitch; //Apply yaw as global rotation to pitch
			_camera->Object.Transform.SetRotation(newCameraRotation);

		}
	}

	_lastMouseX = currentMouseX;
	_lastMouseY = currentMouseY;

	return didCameraMove;
}
