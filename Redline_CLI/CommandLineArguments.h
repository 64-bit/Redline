#pragma once
#include <string>
#include "Renderer/FrameOutputSettings.h"

namespace Redline
{
	class RenderApplication;

	class CommandLineArguments
	{
	public:

		//Arguments that may be set from the command line

		std::string SceneFilename;

		bool WriteImageOnRenderFinish;
		std::string OutputFilename;

		int Threads;

		bool EnableFlightCamera;
		bool EnableRenderPreview;

		FrameOutputSettings OutputSettings;
		FrameQuailtySettings QuailtySettings;

		/*
		 * Parse the command line arguments specified by args and argc.
		 * Place a instance of the parsed command line arguments object in the pointer specified in parsedArgumentsOut
		 * Returns 0 if the args are ok, else return a error code
		 */
		static int ParseCommandLineArguments(int argc, char** argv, CommandLineArguments*& parsedArgumentsOut);

	private:
		CommandLineArguments();

		int ParseArguments(int argc, char** argv);

		int ValidateArguments() const;

		int HandleHelpFlag();
	};
}
