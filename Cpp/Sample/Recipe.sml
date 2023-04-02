Name: "Cpp.Sample"
Language: "C++"
Version: "1.0.0"
Interface: "MyClass.cpp"

Tests: {
	Source: [
		"MyClass.UnitTests.cpp"
	]
}

Dependencies: {
	Runtime: [
		"../HelperDLL/"
	]
	Build: [
		"../TestBuild/"
	]
}