// <copyright file="TestBuildTask.wren" company="Soup">
// Copyright (c) Soup. All rights reserved.
// </copyright>

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

	/// <summary>
	/// The Core Execute task
	/// </summary>
	static evaluate() {
		var activeState = Soup.activeState
		var sharedState = Soup.sharedState

		var recipeTable = activeState["Recipe"]
		var activeBuildTable = activeState["Build"]
		var parametersTable = activeState["Parameters"]

		if (!recipeTable.ContainsKey("Tests"))
		{
			throw new InvalidOperationException("No Tests Specified")
		}

		var arguments = new BuildArguments()
		arguments.TargetArchitecture = parametersTable["Architecture"].AsString()

		// Load up the common build properties from the original Build table in the active state
		LoadBuildProperties(activeBuildTable, arguments)

		// Load the test properties
		var testTable = recipeTable["Tests"]
		LoadTestBuildProperties(testTable, arguments)

		// Load up the input build parameters from the shared build state as if
		// this is a dependency build
		var sharedBuildTable = sharedState["Build"]
		LoadDependencyBuildInput(sharedBuildTable, arguments)

		// Load up the test dependencies build input to add extra test runtime libraries
		LoadTestDependencyBuildInput(buildState, activeState, arguments)

		// Update to place the output in a sub folder
		arguments.ObjectDirectory = arguments.ObjectDirectory + Path.new("Test/")
		arguments.BinaryDirectory = arguments.BinaryDirectory + Path.new("Test/")

		// Initialize the compiler to use
		var compilerName = parametersTable["Compiler"].AsString()
		if (!this.compilerFactory.TryGetValue(compilerName, out var compileFactory)) {
			Fiber.abort("Unknown compiler: %(compilerName)")
		}

		var compiler = compileFactory(activeState)

		var buildEngine = new BuildEngine(compiler)
		var buildResult = buildEngine.Execute(buildState, arguments)

		// Create the operation to run tests during build
		var title = "Run Tests"
		var program = buildResult.TargetFile
		var workingDirectory = arguments.TargetRootDirectory
		var runArguments = ""

		// Ensure that the executable and all runtime dependencies are in place before running tests
		var inputFiles = new List<Path>(buildResult.RuntimeDependencies)
		inputFiles.Add(program)

		// The test should have no output
		var outputFiles = new List<Path>()

		var runTestsOperation =
			new BuildOperation(
				title,
				workingDirectory,
				program,
				runArguments,
				inputFiles,
				outputFiles)

		// Run the test harness
		buildResult.BuildOperations.Add(runTestsOperation)

		// Register the build operations
		for (operation in buildResult.BuildOperations) {
			buildState.CreateOperation(operation)
		}
	}

	static createMSVCCompiler {
		return Fn.new { |activeState|
			var clToolPath = Path.new(activeState["MSVC.ClToolPath"])
			var linkToolPath = Path.new(activeState["MSVC.LinkToolPath"])
			var libToolPath = Path.new(activeState["MSVC.LibToolPath"])
			var rcToolPath = Path.new(activeState["MSVC.RCToolPath"])
			var mlToolPath = Path.new(activeState["MSVC.MLToolPath"])
			return MSVCCompiler.new(
				clToolPath,
				linkToolPath,
				libToolPath,
				rcToolPath,
				mlToolPath)
		}
	}

	static LoadBuildProperties(buildTable, arguments)
	{
		arguments.LanguageStandard = (LanguageStandard)
			buildTable["LanguageStandard"].AsInteger()
		arguments.SourceRootDirectory = Path.new(buildTable["SourceRootDirectory"].AsString())
		arguments.TargetRootDirectory = Path.new(buildTable["TargetRootDirectory"].AsString())
		arguments.ObjectDirectory = Path.new(buildTable["ObjectDirectory"].AsString())
		arguments.BinaryDirectory = Path.new(buildTable["BinaryDirectory"].AsString())

		if (buildTable.TryGetValue("IncludeDirectories", out var includeDirectoriesValue))
		{
			arguments.IncludeDirectories = includeDirectoriesValue.AsList().Select(value => Path.new(value.AsString())).ToList()
		}

		if (buildTable.TryGetValue("PlatformLibraries", out var platformLibrariesValue))
		{
			arguments.PlatformLinkDependencies = platformLibrariesValue.AsList().Select(value => Path.new(value.AsString())).ToList()
		}

		if (buildTable.TryGetValue("LinkLibraries", out var linkLibrariesValue))
		{
			arguments.LinkDependencies = linkLibrariesValue.AsList().Select(value => Path.new(value.AsString())).ToList()
		}

		if (buildTable.TryGetValue("LibraryPaths", out var libraryPathsValue))
		{
			arguments.LibraryPaths = libraryPathsValue.AsList().Select(value => Path.new(value.AsString())).ToList()
		}

		if (buildTable.TryGetValue("PreprocessorDefinitions", out var preprocessorDefinitionsValue)) {
			arguments.PreprocessorDefinitions = preprocessorDefinitionsValue.AsList().Select(value => value.AsString()).ToList()
		}

		if (buildTable.TryGetValue("OptimizationLevel", out var optimizationLevelValue)) {
			arguments.OptimizationLevel = (BuildOptimizationLevel)
				optimizationLevelValue.AsInteger()
		} else {
			arguments.OptimizationLevel = BuildOptimizationLevel.None
		}

		if (buildTable.TryGetValue("GenerateSourceDebugInfo", out var generateSourceDebugInfoValue)) {
			arguments.GenerateSourceDebugInfo = generateSourceDebugInfoValue.AsBoolean()
		} else {
			arguments.GenerateSourceDebugInfo = false
		}
	}

	static LoadTestBuildProperties(testTable, arguments) {
		if (testTable.TryGetValue("Source", out var sourceValue)) {
			arguments.SourceFiles = sourceValue.AsList().Select(value => Path.new(value.AsString())).ToList()
		} else {
			Fiber.abort("No Test Source Files")
		}

		// Combine the include paths from the recipe and the system
		if (testTable.TryGetValue("IncludePaths", out var includePathsValue)) {
			arguments.IncludeDirectories = CombineUnique(
				arguments.IncludeDirectories,
				includePathsValue.AsList().Select(value => Path.new(value.AsString())))
		}

		if (testTable.TryGetValue("PlatformLibraries", out var platformLibrariesValue)) {
			arguments.PlatformLinkDependencies = CombineUnique(
				arguments.PlatformLinkDependencies,
				platformLibrariesValue.AsList().Select(value => Path.new(value.AsString())))
		}

		arguments.TargetName = "TestHarness"
		arguments.TargetType = BuildTargetType.Executable
	}

	static LoadDependencyBuildInput(sharedBuildTable, arguments)
	{
		// Load the runtime dependencies
		if (sharedBuildTable.TryGetValue("RuntimeDependencies", out var runtimeDependenciesValue)) {
			arguments.RuntimeDependencies = CombineUnique(
				arguments.RuntimeDependencies,
				runtimeDependenciesValue.AsList().Select(value => Path.new(value.AsString())))
		}

		// Load the link dependencies
		if (sharedBuildTable.TryGetValue("LinkDependencies", out var linkDependenciesValue)) {
			arguments.LinkDependencies = CombineUnique(
				arguments.LinkDependencies,
				linkDependenciesValue.AsList().Select(value => Path.new(value.AsString())))
		}

		// Load the module references
		if (sharedBuildTable.TryGetValue("ModuleDependencies", out var moduleDependenciesValue)) {
			arguments.ModuleDependencies = CombineUnique(
				arguments.ModuleDependencies,
				moduleDependenciesValue.AsList().Select(value => Path.new(value.AsString())))
		}
	}

	static LoadTestDependencyBuildInput(activeState, arguments)
	{
		if (activeState.TryGetValue("Dependencies", out var dependenciesValue)) {
			var dependenciesTable = dependenciesValue
			if (dependenciesTable.TryGetValue("Test", out var testValue)) {
				var testDependenciesTable = testValue
				foreach (var dependencyName in testDependenciesTable) {
					// Combine the core dependency build inputs for the core build task
					buildState.LogTrace(TraceLevel.Information, "Combine Test Dependency: " + dependencyName.Key)
					var dependencyTable = dependencyName.Value

					if (dependencyTable.TryGetValue("Build", out var buildValue)) {
						var dependencyBuildTable = buildValue

						if (dependencyBuildTable.TryGetValue("ModuleDependencies", out var moduleDependenciesValue)) {
							arguments.ModuleDependencies = CombineUnique(
								arguments.ModuleDependencies,
								moduleDependenciesValue.AsList().Select(value => Path.new(value.AsString())))
						}

						if (dependencyBuildTable.TryGetValue("RuntimeDependencies", out var runtimeDependenciesValue)) {
							arguments.RuntimeDependencies = CombineUnique(
								arguments.RuntimeDependencies,
								runtimeDependenciesValue.AsList().Select(value => Path.new(value.AsString())))
						}

						if (dependencyBuildTable.TryGetValue("LinkDependencies", out var linkDependenciesValue)) {
							arguments.LinkDependencies = CombineUnique(
								arguments.LinkDependencies,
								linkDependenciesValue.AsList().Select(value => Path.new(value.AsString())))
						}
					}
				}
			}
		}
	}

	static CombineUnique(collection1, collection2) {
		var valueSet = new HashSet<string>()
		foreach (var value in collection1)
			valueSet.Add(value.ToString())
		foreach (var value in collection2)
			valueSet.Add(value.ToString())

		return valueSet.Select(value => Path.new(value)).ToList()
	}

	static MakeUnique(collection) {
		var valueSet = new HashSet<string>()
		foreach (var value in collection)
			valueSet.Add(value.ToString())

		return valueSet.Select(value => Path.new(value)).ToList()
	}
}
