#pragma once

#include <SDL.h>
#undef main

namespace Redline
{
	class CommandLineArguments;
	class Bitmap2D;

	class RenderPreviewWindow
	{
	public:

		RenderPreviewWindow();

		int CreateWindow(const CommandLineArguments* const arguments);

		void UpdateWindowFromBitmap(std::shared_ptr<Bitmap2D> bitmapToDisplay);

		bool ShouldQuitThisFrame();

	private:

		SDL_Window* _sdlWindow = nullptr;
		SDL_Surface* _sdlWindowSurface;
	};
}
