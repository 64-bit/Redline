#include "stdafx.h"
#include "RenderApplication.h"
#include "CLIErrorCodes.h"
#include "CommandLineArguments.h"
#include "CameraFlightController.h"
#include "Renderer/BRDF/CookTorrence/Functions/GGX_Distribution.h"
#include "Renderer/BRDF/CookTorrence/Functions/GGX_Geometry.h"
#include "Renderer/BRDF/CookTorrence/Functions/Schlick_Fresnel.h"

using namespace Redline;

int RenderApplication::Run(const CommandLineArguments* const arguments)
{
	_threadpool = new Threadpool(arguments->Threads, Normal);

	const auto loadSceneResult = LoadScene(arguments);
	if(loadSceneResult != CLI_ERRORCODE___OK)
	{
		return loadSceneResult;
	}

	SDL_Init(SDL_INIT_EVERYTHING);

	SetupFeatures(arguments);

	RunWholeApplication(arguments);

	return CLI_ERRORCODE___OK;
}

void RenderApplication::RestartRender()
{
	//If we have no preview window, render mip level 1, else 16
	_currentRenderMipLevel = _previewWindow == nullptr ? 1 : 16;

	//_currentRenderMipLevel = 1;

	if (_frameResultPromis != nullptr)
	{
		_frameRenderer->StopRendering();
		_frameResultPromis->Getvalue();
	}

	_frameRenderer->ResetRenderer();
	_frameResultPromis = _frameRenderer->RenderFrameAsync(_currentRenderMipLevel);
}

RenderApplication::~RenderApplication()
{
	delete _threadpool;
}

//Temp
#include "Scene/Scene.h"
#include "Renderer/Raytracer.h"

#include <ostream>
#include <iostream>

#include "SDL.h"
#undef main

#include "Scene/Components/Renderable/SphereRenderer.h"
#include "Scene/Components/CameraComponent.h"

#include "Renderer/FrameRenderer.h"
#include "GraphicalResources/CubemapTexture.h"
#include "Utilities/Stopwatch.h"
#include "Renderer/BRDF/CookTorrence/CookTorrence.h"
#include "FileFormats/GLTFSceneLoader.h"

using namespace std;
using namespace mathfu;


void RenderApplication::RunWholeApplication(const CommandLineArguments* const arguments)
{
	//Start SDL

	//Main loop flag
	bool quit = false;

	int lastTick = SDL_GetTicks();

	//Render frame async
	printf("\n---Raytracing Starting---\n");

	Stopwatch watch;
	RestartRender();

	while (!quit)
	{
		const int thisTick = SDL_GetTicks();
		const int ticksPassed = thisTick - lastTick;
		lastTick = thisTick;
		const float deltaT = static_cast<float>(ticksPassed) / 1000.0f;//Convert miliseconds to seconds

		//Check if the user has pressed escape or click the X on the preview window
		if(_previewWindow != nullptr)
		{
			quit = quit || _previewWindow->ShouldQuitThisFrame();
		}

		//Update this value to true if OnSDLUpdate returns that we moved
		//This lets us move the camera with percisoin, but only restart the render every so often
		if (_flightController != nullptr)
		{
			const bool shouldUpdatePreview = _flightController->OnSDLUpdate(deltaT);
			if(shouldUpdatePreview)
			{
				watch = Stopwatch();
				RestartRender();
			}
		}

		//Check promis for updates and update screen if we are done
		if (_frameResultPromis != nullptr)
		{
			if (_frameResultPromis->IsFufilled())
			{

				if (_previewWindow != nullptr)
				{
					_previewWindow->UpdateWindowFromBitmap(_frameResultPromis->Getvalue());
				}

				printf("\n---Raytracing finished---\n");
				//printf("Raytracing scene took %.1f1 mS\n", raytraceTime.GetMiliseconds());

				if (_currentRenderMipLevel != 1)
				{
					_frameResultPromis = nullptr;

					_currentRenderMipLevel /= 2;
					_frameRenderer->ResetRenderer();
					_frameResultPromis = _frameRenderer->RenderFrameAsync(_currentRenderMipLevel);
				}
				else
				{
					//Stop timer
					watch.Stop();

					auto ms = watch.GetMiliseconds();
					printf("\n---Raytracing finished in %f mS---\n", ms);

					//Write out to disk if enabled.
					//TODO:Better name for multiple preview renders, or don't write this if the camera moved
					if(arguments->WriteImageOnRenderFinish)
					{
						auto bitmap = _frameResultPromis->Getvalue();
						stbi_write_png(arguments->OutputFilename.c_str(), bitmap->Width, bitmap->Height, 4, bitmap->Pixels, bitmap->Width * 4);
						_frameResultPromis = nullptr;
					}

				}

			}
			else
			{
				if(_previewWindow != nullptr)
				{
					_previewWindow->UpdateWindowFromBitmap(_frameRenderer->GetCurrentFrameState());
				}
			}
		}

		//Wiat a minimum of 5 miliseconds between iterations of this loop
		if (ticksPassed < 50)
		{
			int delay = 50 - ticksPassed;
			SDL_Delay(delay);
		}
	}

	//Quit SDL
	SDL_Quit();
	return;
}

int RenderApplication::LoadScene(const CommandLineArguments* const arguments)
{
	GLTFSceneLoader gltfSceneLoader;
	_scene = gltfSceneLoader.LoadSceneFromGLTFFile(arguments->SceneFilename);

	if(_scene == nullptr)
	{		
		printf("Error: Invalid scene\n");
		return CLI_ERRORCODE___INVALID_SCENE;
	}

#ifdef NDEBUG
	//TODO: Read from scene file extension
	//_scene->EnvironmentCubemap = LoadCubemap();
	_scene->BackgroundColor = mathfu::vec3(1.0f, 1.0f, 1.0f);
	_scene->EnvironmentPower = 1.0;
#endif

	//_scene->BackgroundColor = mathfu::vec3(0.369f, 0.438f, 0.5f);
	_scene->BackgroundColor = mathfu::vec3(0.0f, 0.0f, 0.0f);

	_scene->EnvironmentPower = 0.0f;

	_camera = _scene->FindComponentByType<CameraComponent>();
	if(_camera == nullptr)
	{
		printf("Error: No camera found in scene\n");
		return CLI_ERRORCODE___NO_CAMERA_IN_SCENE;
	}

	return CLI_ERRORCODE___OK;
}

int RenderApplication::SetupFeatures(const CommandLineArguments* const arguments)
{
	if(arguments->EnableFlightCamera)
	{
		if(arguments->EnableRenderPreview)
		{
			_flightController = make_shared<CameraFlightController>(*this, _camera);
		}
		else
		{
			printf("Warning: flight camera cannot be enabled without render preview\n");
		}
	}

	if(arguments->EnableRenderPreview)
	{
		_previewWindow = make_shared<RenderPreviewWindow>();
		const int createwWindowResult = _previewWindow->CreateWindow(arguments);
		if(createwWindowResult != CLI_ERRORCODE___OK)
		{
			return createwWindowResult;
		}
	}

	//Create functions for Cook-Torrence
	auto distributionFunction = std::make_shared<CookTorrenceBRDF::GGX_Distribution>();
	auto geometryFunction = std::make_shared<CookTorrenceBRDF::GGX_Geometry>();
	auto fresnelFunction = std::make_shared<CookTorrenceBRDF::Schlick_Fresnel>();

	_brdf = std::make_shared<CookTorrence>(
		distributionFunction,
		geometryFunction,
		fresnelFunction);

	_pathTracer = std::make_shared<PathTracer>(
		arguments->OutputSettings,
		arguments->QuailtySettings,
		_brdf);

	_frameRenderer = make_shared<FrameRenderer>(
		_scene,
		_camera,
		arguments->OutputSettings,
		arguments->QuailtySettings,
		_threadpool,
		_brdf,
		_pathTracer);

	return CLI_ERRORCODE___OK;
}

shared_ptr<CubemapTexture> RenderApplication::LoadCubemap() //TODO:Make this driven by a command line argument and or scene extension
{
	auto cubemap = std::make_shared<CubemapTexture>();

	cubemap->Faces[0] = Bitmap2D::LoadFromFile("TestFiles/cubemap/rightImage.png");
	cubemap->Faces[1] = Bitmap2D::LoadFromFile("TestFiles/cubemap/leftImage.png");

	cubemap->Faces[2] = Bitmap2D::LoadFromFile("TestFiles/cubemap/downImage.png");
	cubemap->Faces[3] = Bitmap2D::LoadFromFile("TestFiles/cubemap/upImage.png");

	cubemap->Faces[4] = Bitmap2D::LoadFromFile("TestFiles/cubemap/backImage.png");
	cubemap->Faces[5] = Bitmap2D::LoadFromFile("TestFiles/cubemap/frontImage.png");

	return cubemap;
}