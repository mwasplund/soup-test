Version: 4
Closures: {
	Root: {
		"C++": [
			{ Name: "Cpp.Sample", Version: "./", Build: "Build0", Tool: "Tool0" }
			{ Name: "Cpp.Sample.Helper", Version: "../HelperDLL/", Build: "Build0", Tool: "Tool0" }
		]
	}
	Build0: {
		Wren: [
			{ Name: "Soup.Test.Cpp", Version: "../TestBuild/" }
			{ Name: "Soup.Cpp", Version: "0.7.0" }
		]
	}
	Tool0: {
		"C++": [
			{ Name: "copy", Version: "1.0.0" }
			{ Name: "mkdir", Version: "1.0.0" }
		]
	}
}