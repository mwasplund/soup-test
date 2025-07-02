Name: 'Cpp.Sample'
Language: 'C++|0'
Version: 1.0.0
Source: [
	'my-class.cpp'
]
Tests: {
	Source: [
		'my-class.tests.cpp'
	]
}
Dependencies: {
	Runtime: [
		'../helper-dll/'
	]
	Build: [
		'../test-build/'
	]
}