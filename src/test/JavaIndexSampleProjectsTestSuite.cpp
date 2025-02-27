#include "Catch2.hpp"

#include "language_packages.h"

#if BUILD_JAVA_LANGUAGE_PACKAGE

#	include <fstream>

#	include "ApplicationSettings.h"
#	include "IndexerCommandJava.h"
#	include "JavaEnvironmentFactory.h"
#	include "JavaParser.h"
#	include "ParserClientImpl.h"
#	include "TestStorage.h"
#	include "TextAccess.h"
#	include "TimeStamp.h"
#	include "utilityJava.h"
#	include "utilityPathDetection.h"
#	include "utilityString.h"

namespace
{
const bool updateExpectedOutput = false;
const bool trackTime = true;
size_t duration;

void setupJavaEnvironmentFactory()
{
	utility::prepareJavaEnvironment(FilePath(L"../app/data/java/"));
}

std::shared_ptr<TextAccess> parseCode(
	const FilePath& sourceFilePath,
	const FilePath& /*projectDataSrcRoot*/,
	const std::vector<FilePath>& classpath)
{
	std::shared_ptr<IntermediateStorage> storage = std::make_shared<IntermediateStorage>();
	JavaParser parser(
		std::make_shared<ParserClientImpl>(storage.get()), std::make_shared<IndexerStateInfo>());
	std::shared_ptr<IndexerCommandJava> command = std::make_shared<IndexerCommandJava>(
		sourceFilePath, L"12", classpath);

	TimeStamp startTime = TimeStamp::now();
	parser.buildIndex(command);
	duration += TimeStamp::now().deltaMS(startTime);

	return TextAccess::createFromLines(TestStorage::create(storage)->m_lines);
}

void processSourceFile(const std::string& projectName, const FilePath& sourceFilePath,
	const std::vector<FilePath>& classpath)
{
	const FilePath projectDataRoot = FilePath("data/JavaIndexSampleProjectsTestSuite/" + projectName);
	const FilePath projectDataSrcRoot = projectDataRoot.getConcatenated(L"src");
	const FilePath projectDataExpectedOutputRoot = projectDataRoot.getConcatenated(L"expected_output");

	std::shared_ptr<TextAccess> output = parseCode(projectDataSrcRoot.getConcatenated(sourceFilePath), projectDataSrcRoot, classpath);

	const FilePath expectedOutputFilePath = projectDataExpectedOutputRoot.getConcatenated(
		utility::replace(sourceFilePath.withoutExtension().wstr() + L".txt", L"/", L"_"));
	if (updateExpectedOutput || !expectedOutputFilePath.exists())
	{
		std::ofstream expectedOutputFile;
		expectedOutputFile.open(expectedOutputFilePath.str());
		expectedOutputFile << output->getText();
		expectedOutputFile.close();
	}
	else
	{
		std::shared_ptr<TextAccess> expectedOutput = TextAccess::createFromFile(expectedOutputFilePath);
		REQUIRE_MESSAGE(("Line count of output '" + sourceFilePath.str() + "' is different than expected output '" + expectedOutputFilePath.str() + "' in project " + projectName + ".").c_str(),
			expectedOutput->getLineCount() == output->getLineCount());

		if (expectedOutput->getLineCount() == output->getLineCount())
		{
			for (unsigned int i = 1; i <= expectedOutput->getLineCount(); i++)
			{
				REQUIRE(expectedOutput->getLine(i) == output->getLine(i));
			}
		}
	}
}

void processSourceFiles(const std::string& projectName, const std::vector<FilePath>& sourceFilePaths,
	const std::vector<FilePath>& classpath)
{
	duration = 0;
	for (const FilePath& filePath: sourceFilePaths)
	{
		processSourceFile(projectName, filePath, classpath);
	}
	if (trackTime)
	{
		const FilePath projectDataRoot =
			FilePath("data/JavaIndexSampleProjectsTestSuite/" + projectName).makeAbsolute();

		std::ofstream outfile;
		outfile.open(
			FilePath(projectDataRoot.str() + "/" + projectName + ".timing").str(),
			std::ios_base::app);
		outfile << TimeStamp::now().toString() << " - " << duration << " ms\n";
		outfile.close();
	}
}
}	 // namespace

TEST_CASE("java sample parser can setup environment factory")
{
	std::vector<FilePath> javaPaths = utility::getJavaRuntimePathDetector()->getPaths();
	if (!javaPaths.empty())
	{
		ApplicationSettings::getInstance()->setJavaPath(javaPaths[0]);
	}

	setupJavaEnvironmentFactory();

	// if this one fails, maybe your java_path in the test settings is wrong.
	REQUIRE(JavaEnvironmentFactory::getInstance().use_count() >= 1);
}

TEST_CASE("index javasymbolsolver 0 6 0 project", JAVA_TAG)
{
	const std::vector<FilePath> classPath = {
		FilePath(L"data/JavaIndexSampleProjectsTestSuite/JavaSymbolSolver060/lib/guava-21.0.jar").makeAbsolute(),
		FilePath(L"data/JavaIndexSampleProjectsTestSuite/JavaSymbolSolver060/lib/javaparser-core-3.3.0.jar").makeAbsolute(),
		FilePath(L"data/JavaIndexSampleProjectsTestSuite/JavaSymbolSolver060/lib/javaslang-2.0.3.jar").makeAbsolute(),
		FilePath(L"data/JavaIndexSampleProjectsTestSuite/JavaSymbolSolver060/lib/javassist-3.19.0-GA.jar").makeAbsolute(),
		FilePath(L"data/JavaIndexSampleProjectsTestSuite/JavaSymbolSolver060/src/java-symbol-solver-core").makeAbsolute(),
		FilePath(L"data/JavaIndexSampleProjectsTestSuite/JavaSymbolSolver060/src/java-symbol-solver-logic").makeAbsolute(),
		FilePath(L"data/JavaIndexSampleProjectsTestSuite/JavaSymbolSolver060/src/java-symbol-solver-model").makeAbsolute()
	};

	const std::vector<FilePath> sourceFilePaths = {
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/SourceFileInfoExtractor.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/core/resolution/Context.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/core/resolution/ContextHelper.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/declarations/common/MethodDeclarationCommonLogic.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparser/Navigator.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparser/package-info.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/DefaultVisitorAdapter.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/JavaParserFacade.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/JavaParserFactory.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/LambdaArgumentTypePlaceholder.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/package-info.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/TypeExtractor.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/UnsolvedSymbolException.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/AbstractJavaParserContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/AbstractMethodLikeDeclarationContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/AnonymousClassDeclarationContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/CatchClauseContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/ClassOrInterfaceDeclarationContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/CompilationUnitContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/ConstructorContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/ContextHelper.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/EnumDeclarationContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/FieldAccessContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/ForechStatementContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/ForStatementContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/JavaParserTypeDeclarationAdapter.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/LambdaExprContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/MethodCallExprContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/MethodContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/StatementContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/SwitchEntryContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/contexts/TryWithResourceContext.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/DefaultConstructorDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/Helper.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserAnnotationDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserAnonymousClassDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserClassDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserConstructorDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserEnumConstantDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserEnumDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserFieldDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserInterfaceDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserMethodDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserParameterDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserSymbolDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserTypeAdapter.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserTypeParameter.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarations/JavaParserTypeVariableDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarators/AbstractSymbolDeclarator.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarators/FieldSymbolDeclarator.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarators/NoSymbolDeclarator.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarators/ParameterSymbolDeclarator.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javaparsermodel/declarators/VariableSymbolDeclarator.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/JavassistClassDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/JavassistConstructorDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/JavassistEnumDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/JavassistFactory.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/JavassistFieldDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/JavassistInterfaceDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/JavassistMethodDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/JavassistParameterDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/JavassistTypeDeclarationAdapter.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/JavassistTypeParameter.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/JavassistUtils.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/javassistmodel/package-info.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/model/typesystem/LazyType.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/model/typesystem/ReferenceTypeImpl.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/MyObjectProvider.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/package-info.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/ReflectionClassAdapter.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/ReflectionClassDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/ReflectionConstructorDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/ReflectionEnumDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/ReflectionFactory.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/ReflectionFieldDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/ReflectionInterfaceDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/ReflectionMethodDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/ReflectionMethodResolutionLogic.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/ReflectionParameterDeclaration.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/ReflectionTypeParameter.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/comparators/ClassComparator.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/comparators/MethodComparator.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/reflectionmodel/comparators/ParameterComparator.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/resolution/ConstructorResolutionLogic.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/resolution/MethodResolutionLogic.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/resolution/SymbolDeclarator.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/resolution/SymbolSolver.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/resolution/typesolvers/CombinedTypeSolver.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/resolution/typesolvers/JarTypeSolver.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/resolution/typesolvers/JavaParserTypeSolver.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/resolution/typesolvers/MemoryTypeSolver.java"),
		FilePath(L"java-symbol-solver-core/com/github/javaparser/symbolsolver/resolution/typesolvers/ReflectionTypeSolver.java"),
		FilePath(L"java-symbol-solver-logic/com/github/javaparser/symbolsolver/logic/AbstractClassDeclaration.java"),
		FilePath(L"java-symbol-solver-logic/com/github/javaparser/symbolsolver/logic/AbstractTypeDeclaration.java"),
		FilePath(L"java-symbol-solver-logic/com/github/javaparser/symbolsolver/logic/ConfilictingGenericTypesException.java"),
		FilePath(L"java-symbol-solver-logic/com/github/javaparser/symbolsolver/logic/FunctionalInterfaceLogic.java"),
		FilePath(L"java-symbol-solver-logic/com/github/javaparser/symbolsolver/logic/InferenceContext.java"),
		FilePath(L"java-symbol-solver-logic/com/github/javaparser/symbolsolver/logic/InferenceVariableType.java"),
		FilePath(L"java-symbol-solver-logic/com/github/javaparser/symbolsolver/logic/ObjectProvider.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/AccessLevel.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/AnnotationDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/ClassDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/ConstructorDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/Declaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/EnumDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/FieldDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/HasAccessLevel.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/InterfaceDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/MethodAmbiguityException.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/MethodDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/MethodLikeDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/ParameterDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/ReferenceTypeDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/TypeDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/TypeParameterDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/TypeParametrizable.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/declarations/ValueDeclaration.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/methods/MethodUsage.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/resolution/SymbolReference.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/resolution/TypeSolver.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/resolution/UnsolvedSymbolException.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/resolution/Value.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/ArrayType.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/LambdaConstraintType.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/NullType.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/PrimitiveType.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/ReferenceType.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/Type.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/TypeTransformer.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/TypeVariable.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/VoidType.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/Wildcard.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/parametrization/TypeParametersMap.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/parametrization/TypeParameterValueProvider.java"),
		FilePath(L"java-symbol-solver-model/com/github/javaparser/symbolsolver/model/typesystem/parametrization/TypeParametrized.java")
	};

	processSourceFiles("JavaSymbolSolver060", sourceFilePaths, classPath);
}

#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE
