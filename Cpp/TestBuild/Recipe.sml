Name: "Soup.Test.Cpp"
Language: "Wren|0"
Version: "0.10.0"
Source: [
	"Tasks/TestBuildTask.wren"
]

Dependencies: {
	Runtime: [
		"mwasplund|Soup.Cpp.Compiler@0"
		"mwasplund|Soup.Cpp.Compiler.MSVC@0"
		"mwasplund|Soup.Build.Utils@0"
	]
	Tool: [
		"[C++]mwasplund|copy@1"
		"[C++]mwasplund|mkdir@1"
	]
}