Name: "Soup.Test.Cpp"
Language: "C#|0.1"
Version: "0.4.0"
Source: [
	"Tasks/TestBuildTask.cs"
]

Dependencies: {
	Runtime: [
		{ Reference: "Opal@1.1.0" }
		{ Reference: "Soup.Build@0.2.0", ExcludeRuntime: true }
		{ Reference: "Soup.Build.Extensions@0.4.0" }
		{ Reference: "Soup.Cpp.Compiler@0.5.0" }
		{ Reference: "Soup.Cpp.Compiler.MSVC@0.5.0" }
	]
}