Name: 'Soup.Test.Cpp'
Language: 'Wren|0'
Version: 0.15.1
Source: [
	'tasks/test-build-task.wren'
]
Dependencies: {
	Runtime: [
		'Soup|Cpp.Compiler@0'
		'Soup|Cpp.Compiler.Clang@0'
		'Soup|Cpp.Compiler.GCC@0'
		'Soup|Cpp.Compiler.MSVC@0'
		'Soup|Build.Utils@0'
	]
	Tool: [
		'[C++]mwasplund|copy@1'
		'[C++]mwasplund|mkdir@1'
		'[C++]mwasplund|parse.modules@1'
	]
}