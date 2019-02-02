#include "stdafx.h"
#include "RenderPreviewWindow.h"
#include "CommandLineArguments.h"
#include "CLIErrorCodes.h"
#include <GraphicalResources/Bitmap2D.h>

using namespace Redline;

RenderPreviewWindow::RenderPreviewWindow()
{

}

int RenderPreviewWindow::CreateWindow(const CommandLineArguments* const arguments)
{
	_sdlWindow = SDL_CreateWindow((std::string("Redline - Render Preview - ") + arguments->SceneFilename).c_str(), 100, 100,
		arguments->OutputSettings.OutputWidth, arguments->OutputSettings.OutputHeight, SDL_WINDOW_SHOWN);

	if (_sdlWindow == nullptr)
	{
		printf("Error: SDL could not create window: %s\n", SDL_GetError());
		SDL_Quit();
		return CLI_ERRORCODE___SDL_ERROR;
	}

	_sdlWindowSurface = SDL_GetWindowSurface(_sdlWindow);

	if(_sdlWindowSurface == nullptr)
	{
		printf("Error: SDL could not get window surface: %s\n", SDL_GetError());
		SDL_Quit();
		return CLI_ERRORCODE___SDL_ERROR;
	}

	return CLI_ERRORCODE___OK;
}

void RenderPreviewWindow::UpdateWindowFromBitmap(std::shared_ptr<Bitmap2D> bitmapToDisplay)
{
	//copy frame result to window surface, flipping pixesl for display
	const auto pixelCount = bitmapToDisplay->Width * bitmapToDisplay->Height;

	auto src = reinterpret_cast<unsigned char*>(bitmapToDisplay->Pixels);
	auto dst = static_cast<unsigned char*>(_sdlWindowSurface->pixels);

	for (unsigned i = 0; i < pixelCount; i++)
	{
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
		dst[3] = src[3];

		src += 4;
		dst += 4;
	}

	SDL_UpdateWindowSurface(_sdlWindow);
}

bool RenderPreviewWindow::ShouldQuitThisFrame()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		//User requests quit
		if (e.type == SDL_QUIT)
		{
			return true;
		}

		if (e.type == SDL_KEYDOWN)
		{
			if (e.key.keysym.sym == SDLK_ESCAPE)
			{
				return true;
			}
		}
	}

	return false;
}
