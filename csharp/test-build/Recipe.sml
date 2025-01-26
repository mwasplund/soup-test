Name: 'Soup.Test.CSharp'
Language: 'C#'
Version: 0.2.0
Source: [
	'tasks/TestBuildTask.cs'
]

Dependencies: {
	Runtime = [
		{ Reference: 'Opal@1.1.0' }
		{ Reference: 'Soup.Build@0.2.0', ExcludeRuntime = true }
		{ Reference: 'Soup.Build.Extensions@0.1.8' }
		{ Reference: 'Soup.CSharp.Compiler@0.2.0' }
		{ Reference: 'Soup.CSharp.Compiler.Roslyn@0.1.6' }
	]
}