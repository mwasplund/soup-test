﻿// <copyright file="test-build-task.wren" company="Soup">
// Copyright (c) Soup. All rights reserved.
// </copyright>

import "soup" for Soup, SoupTask
import "Soup|Build.Utils:./build-operation" for BuildOperation
import "Soup|Build.Utils:./path" for Path
import "Soup|Build.Utils:./set" for Set
import "Soup|Build.Utils:./list-extensions" for ListExtensions
import "Soup|Build.Utils:./map-extensions" for MapExtensions
import "Soup|Cpp.Compiler:./build-arguments" for BuildArguments, BuildOptimizationLevel, BuildTargetType, SourceFile
import "Soup|Cpp.Compiler:./build-engine" for BuildEngine
import "Soup|Cpp.Compiler.Clang:./clang-compiler" for ClangCompiler
import "Soup|Cpp.Compiler.GCC:./gcc-compiler" for GCCCompiler
import "Soup|Cpp.Compiler.MSVC:./msvc-compiler" for MSVCCompiler

/// <summary>
/// The test build task that will run after the main build task
/// </summary>
class TestBuildTask is SoupTask {
	/// <summary>
	/// Get the run before list
	/// </summary>
	static runBefore { [] }

	/// <summary>
	/// Get the run after list
	/// </summary>
	static runAfter { [
		"BuildTask",
	] }

	static registerCompiler(name, factory) {
		if (__compilerFactory is Null) __compilerFactory = {}
		__compilerFactory[name] = factory
	}

	/// <summary>
	/// The Core Execute task
	/// </summary>
	static evaluate() {
		// Register default compilers
		TestBuildTask.registerCompiler("Clang", TestBuildTask.createClangCompiler)
		TestBuildTask.registerCompiler("GCC", TestBuildTask.createGCCCompiler)
		TestBuildTask.registerCompiler("MSVC", TestBuildTask.createMSVCCompiler)

		var activeState = Soup.activeState
		var globalState = Soup.globalState
		var sharedState = Soup.sharedState

		var recipe = globalState["Recipe"]
		var activeBuildTable = activeState["Build"]

		if (!recipe.containsKey("Tests")) {
			Fiber.abort("No Tests Specified")
		}

		var arguments = BuildArguments.new()

		// Load up the common build properties from the original Build table in the active state
		TestBuildTask.LoadBuildProperties(activeBuildTable, arguments)

		// Load the test properties
		var tests = recipe["Tests"]
		var preprocessors = globalState["Preprocessors"]
		TestBuildTask.LoadTestBuildProperties(tests, preprocessors, arguments)

		// Load up the input build parameters from the shared build state as if
		// this is a dependency build
		var sharedBuildTable = sharedState["Build"]
		TestBuildTask.LoadDependencyBuildInput(sharedBuildTable, arguments)

		// Load up the test dependencies build input to add extra test runtime libraries
		TestBuildTask.LoadTestDependencyBuildInput(globalState, arguments)

		// Update to place the output in a sub folder
		arguments.ObjectDirectory = arguments.ObjectDirectory + Path.new("Test/")
		arguments.BinaryDirectory = arguments.BinaryDirectory + Path.new("Test/")

		// Initialize the compiler to use
		var compilerName = activeBuildTable["Compiler"]
		Soup.info("Using Compiler: %(compilerName)")
		if (!__compilerFactory.containsKey(compilerName)) {
			Fiber.abort("Unknown compiler: %(compilerName)")
		}

		var compiler = __compilerFactory[compilerName].call(activeState)

		var buildEngine = BuildEngine.new(compiler)
		var buildResult = buildEngine.Execute(arguments)

		// Create the operation to run tests during build
		var title = "Run Tests"
		var program = buildResult.TargetFile
		var workingDirectory = arguments.TargetRootDirectory
		var runArguments = []

		// Ensure that the executable and all runtime dependencies are in place before running tests
		var inputFiles = []
		inputFiles = inputFiles + buildResult.RuntimeDependencies
		inputFiles.add(program)

		// The test should have no output
		var outputFiles = []

		var runTestsOperation = BuildOperation.new(
			title,
			workingDirectory,
			program,
			runArguments,
			inputFiles,
			outputFiles)

		// Run the test harness
		buildResult.BuildOperations.add(runTestsOperation)

		// Register the build operations
		for (operation in buildResult.BuildOperations) {
			Soup.createOperation(
				operation.Title,
				operation.Executable.toString,
				operation.Arguments,
				operation.WorkingDirectory.toString,
				ListExtensions.ConvertFromPathList(operation.DeclaredInput),
				ListExtensions.ConvertFromPathList(operation.DeclaredOutput))
		}

		Soup.info("Test Build Generate Done")
	}

	static createClangCompiler {
		return Fn.new { |activeState|
			Soup.info("%(activeState)")
			var clang = activeState["Clang"]
			var clangToolPath = Path.new(clang["CppCompiler"])
			var archiveToolPath = Path.new(clang["Archiver"])
			return ClangCompiler.new(
				clangToolPath,
				archiveToolPath)
		}
	}

	static createGCCCompiler {
		return Fn.new { |activeState|
			var gcc = activeState["GCC"]
			var gccToolPath = Path.new(gcc["CppCompiler"])
			return GCCCompiler.new(
				gccToolPath)
		}
	}

	static createMSVCCompiler {
		return Fn.new { |activeState|
			var msvc = activeState["MSVC"]
			var clToolPath = Path.new(msvc["ClToolPath"])
			var linkToolPath = Path.new(msvc["LinkToolPath"])
			var libToolPath = Path.new(msvc["LibToolPath"])
			var rcToolPath = Path.new(msvc["RCToolPath"])
			var mlToolPath = Path.new(msvc["MLToolPath"])
			return MSVCCompiler.new(
				clToolPath,
				linkToolPath,
				libToolPath,
				rcToolPath,
				mlToolPath)
		}
	}

	static LoadBuildProperties(buildTable, arguments) {
		arguments.TargetArchitecture = buildTable["Architecture"]
		arguments.LanguageStandard = buildTable["LanguageStandard"]
		arguments.SourceRootDirectory = Path.new(buildTable["SourceRootDirectory"])
		arguments.TargetRootDirectory = Path.new(buildTable["TargetRootDirectory"])
		arguments.ObjectDirectory = Path.new(buildTable["ObjectDirectory"])
		arguments.BinaryDirectory = Path.new(buildTable["BinaryDirectory"])

		if (buildTable.containsKey("IncludeDirectories")) {
			arguments.IncludeDirectories = ListExtensions.ConvertToPathList(buildTable["IncludeDirectories"])
		}

		if (buildTable.containsKey("PlatformLibraries")) {
			arguments.PlatformLinkDependencies = ListExtensions.ConvertToPathList(buildTable["PlatformLibraries"])
		}

		if (buildTable.containsKey("LinkLibraries")) {
			arguments.LinkDependencies = ListExtensions.ConvertToPathList(buildTable["LinkLibraries"])
		}

		if (buildTable.containsKey("LibraryPaths")) {
			arguments.LibraryPaths = ListExtensions.ConvertToPathList(buildTable["LibraryPaths"])
		}

		if (buildTable.containsKey("PreprocessorDefinitions")) {
			arguments.PreprocessorDefinitions = buildTable["PreprocessorDefinitions"]
		}

		if (buildTable.containsKey("OptimizationLevel")) {
			arguments.OptimizationLevel = buildTable["OptimizationLevel"]
		} else {
			arguments.OptimizationLevel = BuildOptimizationLevel.None
		}

		if (buildTable.containsKey("GenerateSourceDebugInfo")) {
			arguments.GenerateSourceDebugInfo = buildTable["GenerateSourceDebugInfo"]
		} else {
			arguments.GenerateSourceDebugInfo = false
		}
	}

	static LoadTestBuildProperties(tests, preprocessors, arguments) {
		if (tests.containsKey("Source")) {
			// Fill in the info on existing source files
			var sourceFiles = ListExtensions.ConvertToPathList(tests["Source"])
			arguments.SourceFiles = TestBuildTask.UpdateCompileFiles(sourceFiles, preprocessors)
		} else {
			Fiber.abort("No Test Source Files")
		}

		// Combine the include paths from the recipe and the system
		if (tests.containsKey("IncludePaths")) {
			arguments.IncludeDirectories = TestBuildTask.CombinePathListUnique(
				arguments.IncludeDirectories,
				ListExtensions.ConvertToPathList(tests["IncludePaths"]))
		}

		if (tests.containsKey("PlatformLibraries")) {
			arguments.PlatformLinkDependencies = TestBuildTask.CombinePathListUnique(
				arguments.PlatformLinkDependencies,
				ListExtensions.ConvertToPathList(tests["PlatformLibraries"]))
		}

		arguments.TargetName = "TestHarness"
		arguments.TargetType = BuildTargetType.Executable
	}

	static UpdateCompileFiles(sourceFiles, preprocessors) {
		Soup.info("Update Files")
		var result = []
		for (sourceFile in sourceFiles) {
			result.add(TestBuildTask.UpdateSourceInfo(sourceFile, preprocessors))
		}

		return result
	}

	static UpdateSourceInfo(file, preprocessors) {
		Soup.info("Update Source File: %(file)")

		var preprocessorResult = TestBuildTask.ResolvePreprocessorResult(file, preprocessors)
		var imports = []
		var module = null
		var isInterface = null
		var partition = null
		for (entry in preprocessorResult["Result"]) {
			var parseResult = entry.split(" ")
			if (parseResult.count == 0) {
				Fiber.abort("Found empty parse result")
			}

			var resultType = parseResult[0]
			if (resultType == "import") {
				if (parseResult.count == 2) {
					imports.add(parseResult[1])
				} else {
					Fiber.abort("Import result must have exactly two values")
				}
			} else if (resultType == "module-implementation") {
				if (parseResult.count == 2) {
					var moduleValue = parseResult[1].split(":")
					isInterface = false
					module = moduleValue[0]
					if (moduleValue.count == 2) {
						partition = moduleValue[1]
					}
				} else {
					Fiber.abort("Module result must have exactly two values")
				}

			} else if (resultType == "module-interface") {
				if (parseResult.count == 2) {
					var moduleValue = parseResult[1].split(":")
					isInterface = true
					module = moduleValue[0]
					if (moduleValue.count == 2) {
						partition = moduleValue[1]
					}
				} else {
					Fiber.abort("Module result must have exactly two values")
				}

			} else {
				Fiber.abort("Unknown parser result type %(resultType)")
			}
		}

		return SourceFile.new(
			file,
			module,
			isInterface,
			partition,
			imports)
	}

	static ResolvePreprocessorResult(file, preprocessors) {
		var preprocessorName = "Scan %(file)"

		Soup.info("Preprocessor: %(preprocessorName)")
		for (preprocessor in preprocessors) {
			if (preprocessor["Title"] == preprocessorName) {
				return preprocessor
			}
		}

		Fiber.abort("Preprocessor result missing for %(file) -> %(preprocessors)")
	}

	static LoadDependencyBuildInput(sharedBuildTable, arguments) {
		// Load the runtime dependencies
		if (sharedBuildTable.containsKey("RuntimeDependencies")) {
			arguments.RuntimeDependencies = TestBuildTask.CombinePathListUnique(
				arguments.RuntimeDependencies,
				ListExtensions.ConvertToPathList(sharedBuildTable["RuntimeDependencies"]))
		}

		// Load the link dependencies
		if (sharedBuildTable.containsKey("LinkDependencies")) {
			arguments.LinkDependencies = TestBuildTask.CombinePathListUnique(
				arguments.LinkDependencies,
				ListExtensions.ConvertToPathList(sharedBuildTable["LinkDependencies"]))
		}

		// Load the module references
		if (sharedBuildTable.containsKey("ModuleDependencies")) {
			arguments.ModuleDependencies = MapExtensions.ConvertToPathMap(sharedBuildTable["ModuleDependencies"])
		}
	}

	static LoadTestDependencyBuildInput(globalState, arguments) {
		if (globalState.containsKey("Dependencies")) {
			var dependenciesTable = globalState["Dependencies"]
			if (dependenciesTable.containsKey("Test")) {
				var testDependenciesTable = dependenciesTable["Test"]
				for (dependencyName in testDependenciesTable) {
					// Combine the core dependency build inputs for the core build task
					Soup.info("Combine Test Dependency: %(dependencyName.key)")
					var dependencyTable = dependencyName.value
					var dependencySharedStateTable = dependencyTable["SharedState"]

					if (dependencySharedStateTable.containsKey("Build")) {
						var dependencyBuildTable = dependencySharedStateTable["Build"]

						if (dependencyBuildTable.containsKey("ModuleDependencies")) {
							TestBuildTask.AddPathMapUnique(
								arguments.ModuleDependencies,
								MapExtensions.ConvertToPathMap(dependencyBuildTable["ModuleDependencies"]))
						}

						if (dependencyBuildTable.containsKey("RuntimeDependencies")) {
							arguments.RuntimeDependencies = TestBuildTask.CombinePathListUnique(
								arguments.RuntimeDependencies,
								ListExtensions.ConvertToPathList(dependencyBuildTable["RuntimeDependencies"]))
						}

						if (dependencyBuildTable.containsKey("LinkDependencies")) {
							arguments.LinkDependencies = TestBuildTask.CombinePathListUnique(
								arguments.LinkDependencies,
								ListExtensions.ConvertToPathList(dependencyBuildTable["LinkDependencies"]))
						}
					}
				}
			}
		}
	}

	static AddPathMapUnique(collection1, collection2) {
		for (value in collection2) {
			// Add the value from the second collection only if not present in the first
			if (!collection1.containsKey(value.key)) {
				collection1[value.key] = value.value
			}
		}
	}

	static CombinePathListUnique(collection1, collection2) {
		var valueSet = Set.new()
		for (value in collection1) {
			valueSet.add(value.toString)
		}
		for (value in collection2) {
			valueSet.add(value.toString)
		}

		return ListExtensions.ConvertToPathList(valueSet.list)
	}

	static MakeUnique(collection) {
		var valueSet = Set.new()
		for (value in collection) {
			valueSet.add(value.toString)
		}

		return ListExtensions.ConvertToPathList(valueSet.list)
	}
}
