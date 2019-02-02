#pragma once
#include "Raytracer.h"
#include "../Scene/Components/CameraComponent.h"
#include "BRDF/BRDF.h"
#include "../Utilities/Multithreading/Promis.h"
#include "../Utilities/Multithreading/Threadpool.h"
#include "RenderTileInfo.h"
#include "GraphicalResources/Bitmap2DFloat.h"
#include "PathTracer.h"

namespace Redline
{
	/*
	 *	Frame renderer class
	 *	Responsible for:
	 *	- taking the following inputs
	 *		A Scene
	 *		A target camera in that scene
	 *		Frame quailty settings struct
	 *		Frame output settings struct
	 *		A fully configured BRDF
	 *	- Creating a threadpool
	 *	- Creating a raytracer (accleration structure owner)
	 *	- Creating a number of ray starts for the given input settings and passing them to threads
	 *	- Gathering results
	 *	- Returning the finished frame
	 *
	 *	This class may render multiple frames, but for now, only the camera may move between them, as other static
	 *	Data structures would need to be rebuilt
	 */

	typedef std::shared_ptr<Promis<std::shared_ptr<Bitmap2D>>> RenderFrameAsyncPromis;

	class FrameRenderer
	{
	public:
		/*
		 * Construct a frame renderer. This is a relativly expensive operation as the Raytracer will build a accleration structure.
		 * While this object is being constructed, changes to the scene on another thread will cause undefined behaviour and must be avoided.
		 * Once this object is constructed, changes to the scene will not affect the final output of the frame
		 */

		FrameRenderer(std::shared_ptr<Redline::Scene>& scene, std::shared_ptr<CameraComponent>& camera,
			const FrameOutputSettings& frameOutputSettings, const FrameQuailtySettings& frameQuailtySettings,
			Threadpool* threadpool, std::shared_ptr<BRDF>& brdf, std::shared_ptr<PathTracer>& pathTracer);

		//Start the process of rendering a frame.
		std::shared_ptr<Promis<std::shared_ptr<Bitmap2D>>> RenderFrameAsync(unsigned renderLowerMip = 1);

		std::shared_ptr<Bitmap2D> GetCurrentFrameState();

		void ProcessPendingTileJob();

		void StopRendering();

		void ResetRenderer();

	private:
		std::shared_ptr<Promis<std::shared_ptr<Bitmap2D>>> _renderPromis;
		std::shared_ptr<Bitmap2D> _currentFrameState;
		std::shared_ptr<Bitmap2DFloat> _floatRenderTarget;

		Threadpool* const _threadpool;
		std::mutex _controlMutex;
		unsigned _totalActiveThreads;
		unsigned _finishedThreads;
		unsigned _currentRenderMipLevel;

		std::vector<RenderTileInfo*> _renderTiles;

		//Priority queue for progressive render
		std::priority_queue<RenderTileInfo*,
		std::vector<RenderTileInfo*>,
		std::function<bool(RenderTileInfo*, RenderTileInfo*)>> _renderTileRenderQueue;

		bool _isRendering = false;
		bool _isStopping = false;
		bool _isDone = false;

		const std::shared_ptr<Scene> _scene;
		const std::shared_ptr<CameraComponent> _targetCamera;
		const FrameOutputSettings _outputSettings;
		const FrameQuailtySettings _quailtySettings;
		const std::shared_ptr<BRDF> _brdf;
		const std::shared_ptr<PathTracer> _pathTracer;

		Raytracer _raytracer;

		SceneLightingData BuildSceneLightingData(const std::shared_ptr<Scene>& scene);

		static bool GetLightsFromSceneObject(SceneObject* sceneObject, SceneLightingData* sceneLightingData);

		//Render entire image on a single thread at once (depreciated)
		void CreateAndRenderRays(std::shared_ptr<Bitmap2D>& outputTarget);

		//Render a slice of pixels and samples
		void CreateAndRenderRaysInRange(std::shared_ptr<Bitmap2D>& outputTarget, const ImageRectangle& imageRegion, unsigned sampleCount);

	};
}
