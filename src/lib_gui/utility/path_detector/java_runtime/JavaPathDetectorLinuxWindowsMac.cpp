#include "JavaPathDetectorLinuxWindowsMac.h"

#include "FilePath.h"
#include "utilityApp.h"

using namespace std;
using namespace utility;

namespace
{
// If those paths change then adjust the place holder hints in QtProjectWizardContentPreferences.cpp!
const wchar_t jvmLibPathRelativeToJavaExecutableLinux[]   = L"/../lib/server/libjvm.so";
const wchar_t jvmLibPathRelativeToJavaExecutableWindows[] = L"/./server/jvm.dll";
const wchar_t jvmLibPathRelativeToJavaExecutableMac[]	  = L"/../lib/server/libjvm.dylib";

FilePath getFilePathRelativeToJavaExecutable(const FilePath& javaExecutablePath)
{
	wstring relativeJvmLibPath;
	switch (getOsType())
	{
		case OsType::OS_LINUX:
			relativeJvmLibPath = jvmLibPathRelativeToJavaExecutableLinux;
			break;
		case OsType::OS_WINDOWS:
			relativeJvmLibPath = jvmLibPathRelativeToJavaExecutableWindows;
			break;
		case OsType::OS_MAC:
			relativeJvmLibPath = jvmLibPathRelativeToJavaExecutableMac;
			break;
	}
	FilePath jvmLibPath = javaExecutablePath.getParentDirectory().concatenate(relativeJvmLibPath);
	if (jvmLibPath.exists())
	{
		return jvmLibPath.makeCanonical();
	}
	return FilePath();
}

FilePath getJavaInPath()
{
	bool ok;
	FilePath javaPath(searchPath(L"java", ok));
	if (ok && !javaPath.empty() && javaPath.exists())
	{
		return javaPath;
	}
	return FilePath();
}

FilePath getJavaInJavaHome()
{
	string command = "";
	char* p = getenv("JAVA_HOME");
	if (p == nullptr)
	{
		return FilePath();
	}

	FilePath javaPath(p);
	javaPath.concatenate(FilePath("/bin/java"));
	if (!javaPath.empty() && javaPath.exists())
	{
		return javaPath;
	}
	return FilePath();
}

FilePath readLink(const FilePath& path)
{
	return path.getCanonical().getAbsolute();
}

}

JavaPathDetectorLinuxWindowsMac::JavaPathDetectorLinuxWindowsMac(const string &name)
	: JavaPathDetector(name)
{
}

vector<FilePath> JavaPathDetectorLinuxWindowsMac::doGetPaths() const
{
	vector<FilePath> paths;
	FilePath p = getJavaInPath();
	if (!p.empty())
	{
		paths.push_back(p);
	}
	p = getJavaInJavaHome();
	if (!p.empty())
	{
		paths.push_back(p);
	}
	for (const FilePath& path: paths)
	{
		FilePath absoluteJavaPath = readLink(path);
		FilePath jvmLibrary = getFilePathRelativeToJavaExecutable(absoluteJavaPath);
		if (jvmLibrary.exists())
		{
			vector<FilePath> foundPath = {jvmLibrary};
			return foundPath;
		}
	}
	return vector<FilePath>();
}

