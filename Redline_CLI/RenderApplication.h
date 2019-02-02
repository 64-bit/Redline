#pragma once
#include "Utilities/Multithreading/Threadpool.h"
#include "Renderer/FrameRenderer.h"
#include "RenderPreviewWindow.h"
#include "Renderer/PathTracer.h"

namespace Redline
{
	class Scene;

	class CommandLineArguments;
	class CubemapTexture;
	class CameraComponent;
	class CameraFlightController;
	class BRDF;
	/*
	 *	A instance of the render application, manages state and provides callbacks for the lifecycle
	 *	of events that can occure during the render
	 *  
	 *  Most command line arguments are implemented by binding listeners to the various callbacks
	 *
	 */
	class RenderApplication
	{
		
	public:

		int Run(const CommandLineArguments* const arguments);

		//Restart the rendering process. This is used when the camera is moved and the render needs to restart
		void RestartRender();

		~RenderApplication();
	private:

		void RunWholeApplication(const CommandLineArguments* const arguments);
		
		int LoadScene(const CommandLineArguments* const arguments);

		int SetupFeatures(const CommandLineArguments* const arguments);

		std::shared_ptr<CubemapTexture> LoadCubemap();

		Threadpool* _threadpool = nullptr;

		std::shared_ptr<BRDF> _brdf;
		std::shared_ptr<PathTracer> _pathTracer;

		std::shared_ptr<FrameRenderer> _frameRenderer;
		RenderFrameAsyncPromis _frameResultPromis;

		std::shared_ptr<Scene> _scene;
		std::shared_ptr<CameraComponent> _camera;

		std::shared_ptr<CameraFlightController> _flightController;
		std::shared_ptr<RenderPreviewWindow> _previewWindow;

		unsigned _currentRenderMipLevel = 16;
	};
}
