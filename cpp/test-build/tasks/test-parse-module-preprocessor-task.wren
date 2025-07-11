// <copyright file="test-parse-module-preprocessor-task.wren" company="Soup">
// Copyright (c) Soup. All rights reserved.
// </copyright>

import "soup" for Soup, SoupPreprocessorTask
import "Soup|Build.Utils:./glob" for Glob
import "Soup|Build.Utils:./path" for Path
import "Soup|Build.Utils:./list-extensions" for ListExtensions
import "Soup|Build.Utils:./shared-operations" for SharedOperations

class TestParseModulePreprocessorTask is SoupPreprocessorTask {
	/// <summary>
	/// The Core Execute task
	/// </summary>
	static evaluate() {
		Soup.info("Finalizer")

		var globalState = Soup.globalState
		var recipe = globalState["Recipe"]
		var tests = recipe["Tests"]

		var allowedPaths = []
		if (tests.containsKey("Source")) {
			// Fill in the info on existing source files
			allowedPaths = ListExtensions.ConvertToPathList(tests["Source"])
		} else {
			// Default to matching all C++ files under the tests folder
			allowedPaths.add(Path.new("./tests/**/*.cpp"))
		}

		// Expand the source from all discovered files
		Soup.info("Expand Source")
		var filesystem = globalState["FileSystem"]
		var sourceFiles = TestParseModulePreprocessorTask.DiscoverCompileFiles(filesystem, Path.new(), allowedPaths)

		var context = globalState["Context"]
		var packageRoot = Path.new(context["PackageDirectory"])
		var targetDirectory = Path.new(context["TargetDirectory"])
		var objectTestsDirectory = Path.new("obj/tests/")

		// Discover the dependency tool
		var parseModuleExecutable = SharedOperations.ResolveRuntimeDependencyRunExecutable("mwasplund|parse.modules")

		// Ensure the output directories exists as the first step
		var createObjectTestsDirectory = SharedOperations.CreateCreateDirectoryOperation(
			targetDirectory,
			objectTestsDirectory)
		Soup.createOperation(
			createObjectTestsDirectory.Title,
			createObjectTestsDirectory.Executable.toString,
			createObjectTestsDirectory.Arguments,
			createObjectTestsDirectory.WorkingDirectory.toString,
			ListExtensions.ConvertFromPathList(createObjectTestsDirectory.DeclaredInput),
			ListExtensions.ConvertFromPathList(createObjectTestsDirectory.DeclaredOutput))

		for (sourceFile in sourceFiles) {
			var targetFile = targetDirectory + objectTestsDirectory + Path.new(sourceFile.GetFileName())
			targetFile.SetFileExtension("txt")

			Soup.createOperation(
				"Scan %(sourceFile)",
				parseModuleExecutable,
				[
					targetFile.toString,
					sourceFile.toString,
				],
				packageRoot.toString,
				[
					sourceFile.toString,
				],
				[
					targetFile.toString,
				])
		}
	}

	static DiscoverCompileFiles(currentDirectory, workingDirectory, allowedPaths) {
		var files = []
		for (directoryEntity in currentDirectory) {
			if (directoryEntity is String) {
				var file = workingDirectory + Path.new(directoryEntity)
				Soup.info("Check File: %(file)")
				if (TestParseModulePreprocessorTask.IsMatchAny(allowedPaths, file)) {
					files.add(file)
				}
			} else {
				for (child in directoryEntity) {
					var directory = workingDirectory + Path.new(child.key)
					Soup.info("Found Directory: %(directory)")
					var subFiles = TestParseModulePreprocessorTask.DiscoverCompileFiles(child.value, directory, allowedPaths)
					ListExtensions.Append(files, subFiles)
				}
			}
		}

		return files
	}

	static IsMatchAny(allowedPaths, file) {
		for (allowedPath in allowedPaths) {
			if (Glob.IsMatch(allowedPath, file)) {
				return true
			}
		}

		return false
	}
}