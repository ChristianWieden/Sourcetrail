#include <boost/filesystem.hpp>
#include <iostream>

#include "ApplicationSettings.h"
#include "language_packages.h"
#include "utilityPathDetection.h"
#include <AppPath.h>
#include <UserPaths.h>
#include <setupApp.h>
#include "Platform.h"

#include "Catch2.hpp"

using namespace std;
using namespace boost::filesystem;
using namespace utility;

struct EventListener : Catch2::EventListenerBase
{
	static int s_argc;
	static char **s_argv;

	using Catch2::EventListenerBase::EventListenerBase;	   // inherit constructor

	void testRunStarting(const Catch::TestRunInfo& ) override
	{
		setupAppDirectories(s_argc, s_argv);

		// The tests assume an empty shared data directory, so unset it:
		AppPath::setSharedDataDirectoryPath(FilePath());

		FilePath settingsFilePath = UserPaths::getAppSettingsFilePath();
		cout << "Loading settings from " << settingsFilePath.str() << endl;
		ApplicationSettings::getInstance()->load(settingsFilePath, true);

#if BUILD_JAVA_LANGUAGE_PACKAGE
		if (ApplicationSettings::getInstance()->getJavaPath().empty())
		{
			shared_ptr<PathDetector> pathDetector = utility::getJavaRuntimePathDetector();
			const vector<FilePath> paths = pathDetector->getPaths();
			if (!paths.empty())
			{
				ApplicationSettings::getInstance()->setJavaPath(paths.front());
				cout << "Java path written to settings: "
						  << ApplicationSettings::getInstance()->getJavaPath().str() << endl;
			}
			else
			{
				cout << "no Java" << endl;
			}
		}
		else
		{
			cout << "Java path read from settings: "
					  << ApplicationSettings::getInstance()->getJavaPath().str() << endl;
		}
#endif
	}
};

int EventListener::s_argc = 0;
char **EventListener::s_argv = nullptr;

CATCH_REGISTER_LISTENER(EventListener)

int main(int argc, char* argv[])
{
	EventListener::s_argc = argc;
	EventListener::s_argv = argv;

	// Workaround for "Unable to configure working directory in CMake/Catch"
	// https://github.com/catchorg/Catch2/issues/2249
	// Set the 'working directory' manually:

	path workingDirectory = absolute(path(argv[0])).parent_path();
	cout << "Set working directory to '" << workingDirectory << "'" << endl;
	current_path(workingDirectory);

	return Catch::Session().run( argc, argv );
}

