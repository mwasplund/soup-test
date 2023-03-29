Name: "Soup.Test.Cpp"
Language: "Wren|0.1"
Version: "0.7.0"
Source: [
	"Tasks/TestBuildTask.wren"
]

Dependencies: {
	Runtime: [
		"Soup.Cpp.Compiler@0.6.1"
		"Soup.Cpp.Compiler.MSVC@0.6.0"
		"Soup.Build.Utils@0.3.0"
	]
	Tool: [
		"C++|copy@1.0.0"
		"C++|mkdir@1.0.0"
	]
}