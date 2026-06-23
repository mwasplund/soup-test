Version: 6
Closure: {
	Wren: {
		'mwasplund|soup-test-cpp': { Version: './', Build: '0', Tool: '0' }
		'soup-test-cpp': { Version: './', Build: '0', Tool: '0' }
		'soup|build-utils': { Version: 0.9.3, Digest: 'sha256:640456fb5c985c2ff460c9b00197811c273d6f093f1fb6695baa3a7bea78d521', Build: '0', Tool: '0' }
		'soup|cpp-compiler': { Version: 0.17.1, Digest: 'sha256:89a9e06482f4d267900c4ecb36737b48442bf6ecf98022e90efd81673d52ce3f', Build: '0', Tool: '0' }
		'soup|cpp-compiler-clang': { Version: 0.9.1, Digest: 'sha256:4ba9fdb1c1af3d5b8ded4381854eeb4929efbc7fa6a3f4f6202fcbee69d16527', Build: '0', Tool: '0' }
		'soup|cpp-compiler-gcc': { Version: 0.8.0, Digest: 'sha256:3f22f75709a8911e7e521e9611c399d2579ab6755e8dbfe92e0d27b346063af4', Build: '0', Tool: '0' }
		'soup|cpp-compiler-msvc': { Version: 0.15.0, Digest: 'sha256:ea97f3ade3c9b08cc41716c71930483aa47c42af554d27cf436c529a8a9677ea', Build: '0', Tool: '0' }
	}
}
Builds: {
	'0': {
		Wren: {
			'soup|wren': {
				Version: 0.6.0
				Digest: 'sha256:b9e3a6552b51220582684f69bb2cb89fdcf364e4fe6ea4b86ab00f51a45f0d7e'
				Artifacts: {
					Linux: 'sha256:e7f6a90708f8b3196a316546ddb7ef3c9b9b0e0c85bb933ae6a90811daf15629'
					Windows: 'sha256:1356ba73d8ecfc5d3f0dd05130b0aa4f94110dd3a8ab48a827b877620dbe7a0e'
				}
			}
		}
	}
}
Tools: {
	'0': {
		'C++': {
			'mwasplund|copy': {
				Version: 1.2.0
				Digest: 'sha256:d493afdc0eba473a7f5a544cc196476a105556210bc18bd6c1ecfff81ba07290'
				Artifacts: {
					Linux: 'sha256:cd2e05f53f8e6515383c6b5b5dc6423bda03ee9d4efe7bd2fa74f447495471d2'
					Windows: 'sha256:c4dc68326a11a704d568052e1ed46bdb3865db8d12b7d6d3e8e8d8d6d3fad6c8'
				}
			}
			'mwasplund|mkdir': {
				Version: 1.2.0
				Digest: 'sha256:b423f7173bb4eb233143f6ca7588955a4c4915f84945db5fb06ba2eec3901352'
				Artifacts: {
					Linux: 'sha256:bbf3cd98e44319844de6e9f21de269adeb0dabf1429accad9be97f3bd6c56bbd'
					Windows: 'sha256:4d43a781ed25ae9a97fa6881da7c24425a3162703df19964d987fb2c7ae46ae3'
				}
			}
		}
	}
}