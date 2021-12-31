// <copyright file="TestBuildTask.cs" company="Soup">
// Copyright (c) Soup. All rights reserved.
// </copyright>

using Opal;
using Soup.Build.CSharp.Compiler;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Soup.Build.CSharp
{
	public class TestBuildTask : IBuildTask
	{
		private IBuildState buildState;
		private IValueFactory factory;
		private IDictionary<string, Func<IValueTable, ICompiler>> compilerFactory;

		/// <summary>
		/// Get the run before list
		/// </summary>
		public static IReadOnlyList<string> RunBeforeList => new List<string>()
		{
		};

		/// <summary>
		/// Get the run after list
		/// </summary>
		public static IReadOnlyList<string> RunAfterList => new List<string>()
		{
			"BuildTask",
		};

		public TestBuildTask(IBuildState buildState, IValueFactory factory) :
			this(buildState, factory, new Dictionary<string, Func<IValueTable, ICompiler>>())
		{
			// Register default compilers
			// TODO: Fix up compiler names for different languages
			this.compilerFactory.Add("MSVC", (IValueTable activeState) =>
			{
				var clToolPath = new Path(activeState["Roslyn.CscToolPath"].AsString());
				return new Compiler.Roslyn.Compiler(clToolPath);
			});
		}

		public TestBuildTask(IBuildState buildState, IValueFactory factory, Dictionary<string, Func<IValueTable, ICompiler>> compilerFactory)
		{
			this.buildState = buildState;
			this.factory = factory;
			this.compilerFactory = compilerFactory;
		}

		public void Execute()
		{
			var activeState = this.buildState.ActiveState;
			var sharedState = this.buildState.SharedState;

			var recipeTable = activeState["Recipe"].AsTable();
			var activeBuildTable = activeState["Build"].AsTable();
			var parametersTable = activeState["Parameters"].AsTable();

			if (!recipeTable.ContainsKey("Tests"))
			{
				throw new InvalidOperationException("No Tests Specified");
			}

			var arguments = new BuildArguments();
			arguments.TargetArchitecture = parametersTable["Architecture"].AsString();

			// Load up the common build properties from the original Build table in the active state
			LoadBuildProperties(activeBuildTable, arguments);

			// Load the test properties
			var testTable = recipeTable["Tests"].AsTable();
			LoadTestBuildProperties(buildState, testTable, arguments);

			// Load up the input build parameters from the shared build state as if
			// this is a dependency build
			var sharedBuildTable = sharedState["Build"].AsTable();
			LoadDependencyBuildInput(sharedBuildTable, arguments);

			// Load up the test dependencies build input to add extra test runtime libraries
			LoadTestDependencyBuildInput(buildState, activeState, arguments);

			// Update to place the output in a sub folder
			arguments.ObjectDirectory = arguments.ObjectDirectory + new Path("Test/");
			arguments.BinaryDirectory = arguments.BinaryDirectory + new Path("Test/");

			// Initialize the compiler to use
			var compilerName = parametersTable["Compiler"].AsString();
			if (!this.compilerFactory.TryGetValue(compilerName, out var compileFactory))
			{
				this.buildState.LogTrace(TraceLevel.Error, "Unknown compiler: " + compilerName);
				throw new InvalidOperationException();
			}

			var compiler = compileFactory(activeState);

			var buildEngine = new BuildEngine(compiler);
			var buildResult = buildEngine.Execute(buildState, arguments);

			// Create the operation to run tests during build
			var title = "Run Tests";
			var program = new Path("C:/Program Files/dotnet/dotnet.exe");
			var runtimeConfigFile = arguments.BinaryDirectory + new Path($"{arguments.TargetName}.runtimeconfig.json");
			var workingDirectory = arguments.TargetRootDirectory;
			var runArguments = buildResult.TargetFile.ToString();

			// Ensure that the executable and all runtime dependencies are in place before running tests
			var inputFiles = new List<Path>(buildResult.RuntimeDependencies);
			inputFiles.Add(program);
			inputFiles.Add(runtimeConfigFile);

			// The test should have no output
			var outputFiles = new List<Path>();

			var runTestsOperation =
				new BuildOperation(
					title,
					workingDirectory,
					program,
					runArguments,
					inputFiles,
					outputFiles);

			// Run the test harness
			buildResult.BuildOperations.Add(runTestsOperation);

			// Register the build operations
			foreach (var operation in buildResult.BuildOperations)
			{
				buildState.CreateOperation(operation);
			}
		}

		void LoadBuildProperties(
			IValueTable buildTable,
			BuildArguments arguments)
		{
			arguments.SourceRootDirectory = new Path(buildTable["SourceRootDirectory"].AsString());
			arguments.TargetRootDirectory = new Path(buildTable["TargetRootDirectory"].AsString());
			arguments.ObjectDirectory = new Path(buildTable["ObjectDirectory"].AsString());
			arguments.BinaryDirectory = new Path(buildTable["BinaryDirectory"].AsString());

			if (buildTable.TryGetValue("LibraryPaths", out var libraryPathsValue))
			{
				arguments.LibraryPaths = libraryPathsValue.AsList().Select(value => new Path(value.AsString())).ToList();
			}

			if (buildTable.TryGetValue("PreprocessorDefinitions", out var preprocessorDefinitionsValue))
			{
				arguments.PreprocessorDefinitions = preprocessorDefinitionsValue.AsList().Select(value => value.AsString()).ToList();
			}

			if (buildTable.TryGetValue("OptimizationLevel", out var optimizationLevelValue))
			{
				arguments.OptimizationLevel = (BuildOptimizationLevel)
					optimizationLevelValue.AsInteger();
			}
			else
			{
				arguments.OptimizationLevel = BuildOptimizationLevel.None;
			}

			if (buildTable.TryGetValue("GenerateSourceDebugInfo", out var generateSourceDebugInfoValue))
			{
				arguments.GenerateSourceDebugInfo = generateSourceDebugInfoValue.AsBoolean();
			}
			else
			{
				arguments.GenerateSourceDebugInfo = false;
			}

			// Load the link dependencies
			if (buildTable.TryGetValue("LinkDependencies", out var linkDependenciesValue))
			{
				arguments.LinkDependencies = CombineUnique(
					arguments.LinkDependencies,
					linkDependenciesValue.AsList().Select(value => new Path(value.AsString())));
			}
		}

		void LoadTestBuildProperties(
			IBuildState buildState,
			IValueTable testTable,
			BuildArguments arguments)
		{
			if (testTable.TryGetValue("Source", out var sourceValue))
			{
				arguments.SourceFiles = sourceValue.AsList().Select(value => new Path(value.AsString())).ToList();
			}
			else
			{
				throw new InvalidOperationException("No Test Source Files");
			}

			arguments.TargetName = "TestHarness";
			arguments.TargetType = BuildTargetType.Executable;
		}

		void LoadDependencyBuildInput(
			IValueTable sharedBuildTable,
			BuildArguments arguments)
		{
			// Load the runtime dependencies
			if (sharedBuildTable.TryGetValue("RuntimeDependencies", out var runtimeDependenciesValue))
			{
				arguments.RuntimeDependencies = MakeUnique(
					runtimeDependenciesValue.AsList().Select(value => new Path(value.AsString())));
			}

			// Load the link dependencies
			if (sharedBuildTable.TryGetValue("LinkDependencies", out var linkDependenciesValue))
			{
				arguments.LinkDependencies = CombineUnique(
					arguments.LinkDependencies,
					linkDependenciesValue.AsList().Select(value => new Path(value.AsString())));
			}
		}

		static void LoadTestDependencyBuildInput(
			IBuildState buildState,
			IValueTable activeState,
			BuildArguments arguments)
		{
			if (activeState.TryGetValue("Dependencies", out var dependenciesValue))
			{
				var dependenciesTable = dependenciesValue.AsTable();
				if (dependenciesTable.TryGetValue("Test", out var testValue))
				{
					var testDependenciesTable = testValue.AsTable();
					foreach (var dependencyName in testDependenciesTable)
					{
						// Combine the core dependency build inputs for the core build task
						buildState.LogTrace(TraceLevel.Information, "Combine Test Dependency: " + dependencyName.Key);
						var dependencyTable = dependencyName.Value.AsTable();

						if (dependencyTable.TryGetValue("Build", out var buildValue))
						{
							var dependencyBuildTable = buildValue.AsTable();

							if (dependencyBuildTable.TryGetValue("RuntimeDependencies", out var runtimeDependenciesValue))
							{
								arguments.RuntimeDependencies = CombineUnique(
									arguments.RuntimeDependencies,
									runtimeDependenciesValue.AsList().Select(value => new Path(value.AsString())));
							}

							if (dependencyBuildTable.TryGetValue("LinkDependencies", out var linkDependenciesValue))
							{
								arguments.LinkDependencies = CombineUnique(
									arguments.LinkDependencies,
									linkDependenciesValue.AsList().Select(value => new Path(value.AsString())));
							}
						}
					}
				}
			}
		}

		static List<Path> CombineUnique(
			IEnumerable<Path> collection1,
			IEnumerable<Path> collection2)
		{
			var valueSet = new HashSet<string>();
			foreach (var value in collection1)
				valueSet.Add(value.ToString());
			foreach (var value in collection2)
				valueSet.Add(value.ToString());

			return valueSet.Select(value => new Path(value)).ToList();
		}

		private static List<Path> MakeUnique(IEnumerable<Path> collection)
		{
			var valueSet = new HashSet<string>();
			foreach (var value in collection)
				valueSet.Add(value.ToString());

			return valueSet.Select(value => new Path(value)).ToList();
		}
	}
}
