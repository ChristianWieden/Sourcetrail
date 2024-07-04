#ifndef CLANG_INVOCATION_INFO_H
#define CLANG_INVOCATION_INFO_H

#include <string>


namespace clang::tooling
{
class CompilationDatabase;
}


struct ClangInvocationInfo
{
	static ClangInvocationInfo getClangInvocationString(
		const clang::tooling::CompilationDatabase* compilationDatabase);

	std::string invocation;
	std::string errors;
};

#endif	  // CLANG_INVOCATION_INFO_H
