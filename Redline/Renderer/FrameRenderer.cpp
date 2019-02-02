#include "stdafx.h"
#include "FrameRenderer.h"

#include <Scene/Components/Lights/DirectionalLightComponent.h>
#include <Scene/Components/Lights/PointLightComponent.h>
#include <Scene/Components/Lights/SpotLightComponent.h>
#include <Scene/SceneObject.h>

#include <Utilities/Multithreading/Threadpool.h>
#include <Utilities/Multithreading/ThreadLocalRandom.h>
#include <Utilities/Stopwatch.h>

#include <random>
#include "RenderPendingQueueJob.h"
#include "Math/Sampling.h"
#include <CompileTimeSettings.h>

using namespace Redline;
using namespace std;
using namespace mathfu;


FrameRenderer::FrameRenderer(shared_ptr<Scene>& scene, shared_ptr<CameraComponent>& camera,
	const FrameOutputSettings& frameOutputSettings, const FrameQuailtySettings& frameQuailtySettings,
	Threadpool* threadpool, shared_ptr<BRDF>& brdf, std::shared_ptr<PathTracer>& pathTracer)
	:
	_threadpool(threadpool),
	_scene(scene),
	_targetCamera(camera),
	_outputSettings(frameOutputSettings),
	_quailtySettings(frameQuailtySettings),
	_brdf(brdf),
	_pathTracer(pathTracer)
{
	assert(scene != nullptr);
	assert(camera != nullptr);
	assert(brdf != nullptr);

	Stopwatch renderSetupTime;

	_renderTileRenderQueue = std::priority_queue<RenderTileInfo*,
		std::vector<RenderTileInfo*>,
		std::function<bool(RenderTileInfo*, RenderTileInfo*)>>(RenderTileInfo::CompareTiles);

	if (scene == nullptr ||
		brdf == nullptr ||
		camera == nullptr)
	{
		//Invalid frame, handle this case
		//TODO: Make a FrameRendererFactory
	}

	printf("Building accleration structure for scene %s", _scene->Name.c_str());
	_raytracer.BuildSceneStructure(_scene);

	//Get directional lights.
	//TODO: Do more design around who is responsible for things like lights, where do they live, and how do they get
	//to the BRDF
	//Would it make sense for the Raytracer structure to also hold information about light visability, or will this
	//not be a huge performance hit. TODO: Profile and find out if light visability is a performance issue
	auto sceneLightingData = BuildSceneLightingData(_scene);

	_pathTracer->SetSceneData(_raytracer, sceneLightingData);

	renderSetupTime.Stop();
	printf("Raytracer scene setup took %.1f mS\n", renderSetupTime.GetMiliseconds());
}

shared_ptr<Promis<shared_ptr<Bitmap2D>>> FrameRenderer::RenderFrameAsync(unsigned renderLowerMip )
{
	lock_guard<mutex> lock(_controlMutex);
	//Check state and if we are ok, set rendering flag to true
	if (_isRendering || _isDone)
	{
		printf("Cannot start rendering, frame has already been started");
	}

	_isRendering = true;
	_isStopping = false;

	//Allocate bitmap
	if (_currentFrameState == nullptr)
	{
		_currentFrameState = make_shared<Bitmap2D>(_outputSettings.OutputWidth, _outputSettings.OutputHeight);
	}

	_currentRenderMipLevel = renderLowerMip;
	const unsigned renderTargetWidth = _outputSettings.OutputWidth / renderLowerMip;
	const unsigned renderTargetHeight = _outputSettings.OutputHeight / renderLowerMip;

	_renderPromis = make_shared<Promis<shared_ptr<Bitmap2D>>>();
	_floatRenderTarget = make_shared<Bitmap2DFloat>(renderTargetWidth, renderTargetHeight);


	//Create ray starts to render frame
	//wait for all rays to finish
	_totalActiveThreads = 0;
	_finishedThreads = 0;

	for (unsigned y = 0; y < renderTargetHeight; y += CompileTimeSettings::BLOCK_SIZE)
	{
		for (unsigned x = 0; x < renderTargetWidth; x += CompileTimeSettings::BLOCK_SIZE)
		{
			auto renderTile = new RenderTileInfo();

			renderTile->ImageRegion.X = x;
			renderTile->ImageRegion.Y = y;
			renderTile->CurrentSamples = 0;

			if (x + CompileTimeSettings::BLOCK_SIZE > renderTargetWidth)
			{
				renderTile->ImageRegion.Width = renderTargetWidth % CompileTimeSettings::BLOCK_SIZE;
			}
			else
			{
				renderTile->ImageRegion.Width = CompileTimeSettings::BLOCK_SIZE;
			}

			if (y + CompileTimeSettings::BLOCK_SIZE > renderTargetHeight)
			{
				renderTile->ImageRegion.Height = renderTargetHeight % CompileTimeSettings::BLOCK_SIZE;
			}
			else
			{
				renderTile->ImageRegion.Height = CompileTimeSettings::BLOCK_SIZE;
			}
			_renderTiles.push_back(renderTile);
			_renderTileRenderQueue.push(renderTile);
		}
	}

	_totalActiveThreads = _threadpool->GetThreadCount();
	for (int i = 0; i < _threadpool->GetThreadCount(); i++)
	{
		//Start all progressive thread jobs
		auto progressiveJob = std::make_shared<RenderPendingQueueJob>();
		progressiveJob->Renderer = this;
		auto castJob = dynamic_pointer_cast<IThreadpoolJob>(progressiveJob);
		_threadpool->RunJob(castJob);
	}

	return _renderPromis;
}

shared_ptr<Bitmap2D> FrameRenderer::GetCurrentFrameState()
{
	return _currentFrameState;
}

void FrameRenderer::ProcessPendingTileJob()
{
	bool hasWork = true;

	while (hasWork)
	{
		RenderTileInfo* tileToImprove = nullptr;
		{
			lock_guard<mutex> lock(_controlMutex);

			if(_isStopping)
			{
				hasWork = false;
				_finishedThreads++;
				if (_finishedThreads == _totalActiveThreads)
				{
					_isDone = true;
					_isRendering = false;
					_renderPromis->Fulfill(_currentFrameState);
				}
				break;
			}

			if (_renderTileRenderQueue.size() > 0)
			{
				tileToImprove = _renderTileRenderQueue.top();
				_renderTileRenderQueue.pop();
			}
			else
			{
				hasWork = false;
				_finishedThreads++;
				if (_finishedThreads == _totalActiveThreads)
				{
					_isDone = true;
					_isRendering = false;
					_renderPromis->Fulfill(_currentFrameState);
				}
				break;
			}
		}

		int samples = CompileTimeSettings::USE_PROGRESSIVE_MODE ? CompileTimeSettings::PROGRESSIVE_IMPROVE_SAMPELS : _outputSettings.SamplesPerPixel;

		if(_currentRenderMipLevel != 1)
		{
			samples = 1;
		}

		//Do work here, outside of lock
		CreateAndRenderRaysInRange(_currentFrameState, tileToImprove->ImageRegion, samples);

		tileToImprove->CurrentSamples += samples;

		//Render tile to output buffer for cool progressive updates
		_floatRenderTarget->MapRegionToBitmap(_currentFrameState, tileToImprove->ImageRegion);

		//Re-Insert tile with additional sampels on it and repeat
		if (tileToImprove->CurrentSamples < _outputSettings.SamplesPerPixel
			&& _currentRenderMipLevel == 1)
		{
			lock_guard<mutex> lock(_controlMutex);
			_renderTileRenderQueue.push(tileToImprove);
		}
	}


}

void FrameRenderer::StopRendering()
{
	_isStopping = true;
}

SceneLightingData FrameRenderer::BuildSceneLightingData(const std::shared_ptr<Scene>& scene)
{
	SceneLightingData lights;

	lights.AmbientLight = scene->AmbientLight;
	lights.BackgroundColor = scene->BackgroundColor;
	lights.EnvironmentCubemap = scene->EnvironmentCubemap;
	lights.EnvironmentPower = scene->EnvironmentPower;

	//Bind GetSurfacesFromSceneObject to this surface list, and then invoke Scene::ForEachSceneObject
	std::function<bool(SceneObject*)> boundCallback =
		std::bind(&FrameRenderer::GetLightsFromSceneObject, std::placeholders::_1, &lights);

	scene->ForEachSceneObject(boundCallback);

	return lights;
}

bool FrameRenderer::GetLightsFromSceneObject(SceneObject* sceneObject,
	SceneLightingData* sceneLightingData)
{
	auto directionalLight = sceneObject->TryGetComponent<DirectionalLightComponent>();
	if (directionalLight != nullptr)
	{
		sceneLightingData->DirectionalLights.push_back(directionalLight->LightSettings);
		sceneLightingData->SceneLights.push_back(static_cast<Light*>(&sceneLightingData->DirectionalLights.back()));
	}

	auto pointLight = sceneObject->TryGetComponent<PointLightComponent>();
	if (pointLight != nullptr)
	{
		auto pointLightData = pointLight->LightSettings;
		pointLightData.Position = sceneObject->Transform.GetPosition();

		sceneLightingData->PointLights.push_back(pointLightData);
		sceneLightingData->SceneLights.push_back(static_cast<Light*>(&sceneLightingData->PointLights.back()));
	}

	auto spotLight = sceneObject->TryGetComponent<SpotLightComponent>();
	if (spotLight != nullptr)
	{
		auto spotLightData = spotLight->LightSettings;
		spotLightData.Position = sceneObject->Transform.GetPosition();
		spotLightData.Direction = sceneObject->Transform.GetRotation() * vec3(0.0f, 0.0f, 1.0f);

		sceneLightingData->SpotLights.push_back(spotLightData);
		sceneLightingData->SceneLights.push_back(static_cast<Light*>(&sceneLightingData->SpotLights.back()));
	}

	return true;
}

void FrameRenderer::CreateAndRenderRays(shared_ptr<Bitmap2D>& outputTarget)
{
	ImageRectangle fullImage;
	fullImage.X = 0;
	fullImage.Y = 0;
	fullImage.Width = outputTarget->Width;
	fullImage.Height = outputTarget->Height;

	CreateAndRenderRaysInRange(outputTarget, fullImage, _outputSettings.SamplesPerPixel);
}

static vec2 OffsetSequence(const vec2& base, const vec2& offset)
{
	float x = fmodf(base.x + offset.x, 1.0f);
	float y = fmodf(base.y + offset.y, 1.0f);
	return vec2(x, y);
}

void FrameRenderer::CreateAndRenderRaysInRange(std::shared_ptr<Bitmap2D>& outputTarget, const ImageRectangle& imageRegion, unsigned sampleCount)
{
	const vec3 cameraPosition = _targetCamera->Object.Transform.GetPosition();
	const quat cameraRotation = _targetCamera->Object.Transform.GetRotation();

	const float fovY = _targetCamera->YAxisFieldofViewRadians;
	const float fovX = fovY * static_cast<float>(outputTarget->Width) / static_cast<float>(outputTarget->Height);

	vec2 seqOffset = vec2(ThreadLocalRandom::NormalFloat(), ThreadLocalRandom::NormalFloat());

	for (unsigned int h = imageRegion.Y; h < imageRegion.Y + imageRegion.Height; h++)
	{
		for (unsigned int w = imageRegion.X; w < imageRegion.X + imageRegion.Width; w++)
		{
			vec3 colorAccum = vec3(0.0f, 0.0f, 0.0f);
			for (unsigned samp = 0; samp < sampleCount; samp++)
			{
				vec2 hammersly = Sampling::Hammersly2D(samp, sampleCount);
				hammersly = OffsetSequence(hammersly, seqOffset);

				//Trace some rays
				ViewRay ray = ViewRay();
				ray.PixelX = w;
				ray.PixelY = h;
				ray.SampleID = samp;
				ray.Origin = cameraPosition;

				float xSSPosition = ((float)w + hammersly.x) / (float)_floatRenderTarget->Width;
				xSSPosition = xSSPosition;
				xSSPosition = (xSSPosition * 2.0f) - 1.0f;

				float ySSPosition = ((float)h + hammersly.y) / (float)_floatRenderTarget->Height;
				ySSPosition = 1.0f - ySSPosition;
				ySSPosition = (ySSPosition * 2.0f) - 1.0f;

				ray.ScreenSpaceOrigin = vec2(xSSPosition, ySSPosition);

				float xDir = xSSPosition * tanf(fovX * 0.5f);
				float yDir = ySSPosition * tanf(fovY * 0.5f);

				ray.Direction = vec3(xDir, yDir, -1.0f).Normalized();
				ray.Direction = cameraRotation * ray.Direction;

				auto rayColor = _pathTracer->ShadeRayStart(ray);
				if(CompileTimeSettings::SAMPLE_CLAMP > 0.0f && rayColor.Length() > CompileTimeSettings::SAMPLE_CLAMP)
				{
					rayColor = rayColor.Normalized() * CompileTimeSettings::SAMPLE_CLAMP;
				}

				_floatRenderTarget->WriteColorSample(w,h, vec4(rayColor,0.0f));
			}
		}
	}
}

void FrameRenderer::ResetRenderer()
{
	lock_guard<mutex> lock(_controlMutex);

	if(_isRendering)
	{
		printf("Cannot reset renderer, threads are still running");
		return;
	}

	_isRendering = false;
	_isStopping = false;
	_isDone = false;

	_totalActiveThreads = 0;
	_finishedThreads = 0;

	//_currentFrameState = nullptr;
	_floatRenderTarget = nullptr;
	_renderPromis = nullptr;
	_renderTiles.clear();

	while(_renderTileRenderQueue.size() > 0)
	{
		_renderTileRenderQueue.pop();
	}
}
