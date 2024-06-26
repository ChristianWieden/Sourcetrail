#ifndef VERSION_H
#define VERSION_H

#include <string>

class Version
{
public:
	static void setApplicationVersion(const Version& version);
	static const Version& getApplicationVersion();

	Version() = default;
	Version(int major, int minor, int patch, int tweak);

	std::string toDisplayString() const;
	std::wstring toDisplayWString() const;

private:
	static Version s_version;

	int m_majorNumber = 0;
	int m_minorNumber = 0;
	int m_patchNumber = 0;
	int m_tweakNumber = 0;
};

#endif	  // VERSION_H
