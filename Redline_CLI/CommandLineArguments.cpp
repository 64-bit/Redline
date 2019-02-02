#include "stdafx.h"
#include "CommandLineArguments.h"
#include "RenderApplication.h"
#include "CLIErrorCodes.h"

#include <functional>
#include <map>

using namespace Redline;
using namespace std;

#ifdef NDEBUG
const int SCALE_FACTOR = 4;
const int SAMPLE_COUNT_FOR_BUILD = 8192;
#else
const int SCALE_FACTOR = 1;
const int SAMPLE_COUNT_FOR_BUILD = 1;
#endif

const int SCREEN_WIDTH = (1920 / 4) * SCALE_FACTOR;
const int SCREEN_HEIGHT = (1080 / 4) * SCALE_FACTOR;

static bool CheckedToInt(char* string, int& intValOut)
{
	char* endOfNum;

	const long parsedValue = strtol(string, &endOfNum, 10);

	if (endOfNum == string)
	{
		return false;
	}

	intValOut = static_cast<int>(parsedValue);
	return true;
}

static bool CheckedToUnsigned(char* string, unsigned& intValOut)
{
	char* endOfNum;

	const long parsedValue = strtol(string, &endOfNum, 10);

	if (endOfNum == string)
	{
		return false;
	}

	intValOut = static_cast<unsigned>(parsedValue);
	return true;
}

class FlagPraser
{
public:
	virtual bool ParseFlagValue(int argc, char** argv, int& currentArgInOut) = 0;
};

class IntFlagParser : public FlagPraser
{
public:
	IntFlagParser(int& target)
		:_target(target) {}

	bool ParseFlagValue(int argc, char** argv, int& currentArgInOut) override
	{
		if(currentArgInOut + 1 >= argc)
		{
			return CLI_ERRORCODE___MISSING_ARGUMENT;
		}
		return CheckedToInt(argv[currentArgInOut++], _target); //Post increment current arg as we consume one
	}

private:
	int& _target;
};

class UnsignedFlagParser : public FlagPraser
{
public:
	UnsignedFlagParser(unsigned& target)
		:_target(target) {}

	bool ParseFlagValue(int argc, char** argv, int& currentArgInOut) override
	{
		if (currentArgInOut + 1 >= argc)
		{
			return CLI_ERRORCODE___MISSING_ARGUMENT;
		}
		return CheckedToUnsigned(argv[currentArgInOut++], _target); //Post increment current arg as we consume one
	}

private:
	unsigned& _target;
};

class StringFlagParser : public FlagPraser
{
public:
	StringFlagParser(string& target)
		:_target(target) {}

	bool ParseFlagValue(int argc, char** argv, int& currentArgInOut) override
	{
		if (currentArgInOut + 1 >= argc)
		{
			return CLI_ERRORCODE___MISSING_ARGUMENT;
		}
		_target = string(argv[currentArgInOut++]); //Post increment current arg as we consume one
		return true;
	}

private:
	string & _target;
};

class BoolFlagEnabler : public FlagPraser
{
public:
	BoolFlagEnabler(bool& target)
		:_target(target) {}

	bool ParseFlagValue(int argc, char** argv, int& currentArgInOut) override
	{
		_target = true;
		return true;
	}

private:
	bool& _target;
};

class BoolFlagDisabler : public FlagPraser
{
public:
	BoolFlagDisabler(bool& target)
		:_target(target) {}

	bool ParseFlagValue(int argc, char** argv, int& currentArgInOut) override
	{
		_target = false;
		return true;
	}

private:
	bool& _target;
};


int CommandLineArguments::ParseCommandLineArguments(int argc, char ** argv, CommandLineArguments*& parsedArgumentsOut)
{
	//Create new instance of arguments and return result of parsing them
	parsedArgumentsOut = new CommandLineArguments();
	return parsedArgumentsOut->ParseArguments(argc, argv);
}

CommandLineArguments::CommandLineArguments()
{
	//Set all default values
#ifdef NDEBUG
	Threads = 0;//default threadcount is auto-detect
#else
	Threads = 1;//Only use 1 thread in debug mode
#endif

	Threads = 24;

	EnableFlightCamera = true;
	EnableRenderPreview = true;

	
	OutputFilename = "out.png";
//	SceneFilename = "TestFiles/gltf/GIBox/gibox3.gltf"; //TODO: Parse from command line arguments instead of using a default value
	WriteImageOnRenderFinish = true;

	OutputSettings.OutputWidth = SCREEN_WIDTH;
	OutputSettings.OutputHeight = SCREEN_HEIGHT;
	OutputSettings.SamplesPerPixel = SAMPLE_COUNT_FOR_BUILD;

	QuailtySettings.EnableShadows = true;
	QuailtySettings.AnalyticLightSamples = 1;
	QuailtySettings.EnableSoftShadows = true;

	QuailtySettings.MaxSpecularBounces = 4;
	QuailtySettings.SpecularReflectionSamples = 1;

	QuailtySettings.MaxDiffuseBounces = 4;
	QuailtySettings.DiffuseReflectionSamples = 1;
}

int CommandLineArguments::ParseArguments(int argc, char** argv)
{
	int currentArg = 1;//Set to one to skip the command used to invoke this

	//Build map of flag parser objects
	map<string, FlagPraser*> flagDecoder =
	{
		{ "-threads", new IntFlagParser(Threads)},
		{ "-width", new UnsignedFlagParser(OutputSettings.OutputWidth) },
		{ "-height", new UnsignedFlagParser(OutputSettings.OutputHeight) },
		{ "-samples", new UnsignedFlagParser(OutputSettings.SamplesPerPixel) },

		{ "-specularBounces", new UnsignedFlagParser(QuailtySettings.MaxSpecularBounces) },
		{ "-diffuseBounces", new UnsignedFlagParser(QuailtySettings.MaxDiffuseBounces) },
		{ "-specularSamples", new UnsignedFlagParser(QuailtySettings.SpecularReflectionSamples) },
		{ "-diffuseSamples", new UnsignedFlagParser(QuailtySettings.DiffuseReflectionSamples) },

		{ "-shadows", new BoolFlagEnabler(QuailtySettings.EnableShadows) },
		{ "-softShadows", new BoolFlagEnabler(QuailtySettings.EnableSoftShadows) },
		{ "-softShadowSamples", new UnsignedFlagParser(QuailtySettings.AnalyticLightSamples) },

		{ "-outputFile", new StringFlagParser(OutputFilename)}
	};

	while (currentArg < argc)
	{
		string currentFlag = string(argv[currentArg++]);//Increment one to account for flag being consumed

		if(currentArg == argc-1)
		{
			//This is the last argument, and thus the scene filename
			SceneFilename = argv[currentArg];
			break;
		}
		else if(currentArg == argc)
		{
			//no scene specified, this is a error, for now use default scene
			SceneFilename = "TestFiles/gltf/GIBox/gibox3.gltf";
		}

		//See if this is found in our flag decoder
		auto it = flagDecoder.find(currentFlag);
		if(it != flagDecoder.end())
		{
			//Calling parseFlagValue will increment currentArg if the flag consumes arguments
			const auto result = it->second->ParseFlagValue(argc, argv, currentArg);
			if (result != CLI_ERRORCODE___OK)
			{
				if(result == CLI_ERRORCODE___MISSING_ARGUMENT)
				{
					printf("Error: value missing for flag %s\n", currentFlag.c_str());
				}
				return result;
			}

			continue;
		}

		//Else check the flags that have special behaviour
		if(currentFlag == "-help")
		{
			return HandleHelpFlag();
		}

	}

	//If soft shadows are enabled, enable regular shadows to ensure shadowing happens
	if (QuailtySettings.EnableSoftShadows)
	{
		QuailtySettings.EnableShadows = true;
	}

	if(OutputFilename.length() > 0)
	{
		WriteImageOnRenderFinish = true;
	}

	//Don't leak temporary objects
	for(auto& pair : flagDecoder)
	{
		delete pair.second;
	}

	return ValidateArguments();
}

int CommandLineArguments::ValidateArguments() const
{
	//Validate settings

	//Check to make sure a scene was specified
	if (SceneFilename.length() == 0)
	{
		printf("Error: no scene specified\n");
		//return CLI_ERRORCODE___NO_SCENE;
	}

	printf("Scene:%s\n", SceneFilename.c_str());
	printf("Threads: %d\n", Threads);

	//No issues, return ok
	return CLI_ERRORCODE___OK;;
}

int CommandLineArguments::HandleHelpFlag()
{
	//TODO:Print help message
	printf("Redline CLI Help message\n");
	return CLI_ERRORCODE___HELP_PRINTED;
}