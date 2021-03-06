#include "stdafx.h"
#include "RenderApplication.h"
#include "CommandLineArguments.h"
#include "CLIErrorCodes.h"

using namespace Redline;

//Main entrypoint into appliction.
//Parse command line arguments and determin if they are valid
//Configure RenderApplication with command line arguments
//Run main loop for application, the behaviour and duration of this will depend on what flags are passed.

int main(int argc, char** argv)
{
	RenderApplication application;
	CommandLineArguments* commandLineArguments;

	const int argsErrorCode = CommandLineArguments::ParseCommandLineArguments(argc, argv, commandLineArguments);

	if(argsErrorCode != CLI_ERRORCODE___OK)
	{
		if(argsErrorCode == CLI_ERRORCODE___HELP_PRINTED)
		{
			return CLI_ERRORCODE___OK;	//This special case causes a normal return if the help message was printed,
										//but prevents the applicatino from running
		}
		return argsErrorCode;//The command line parser will print a detailed error message, just return the code it provided
	}

	const auto applicationResult = application.Run(commandLineArguments);
	delete commandLineArguments;
	return applicationResult;
}