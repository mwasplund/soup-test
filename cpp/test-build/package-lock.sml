Version: 6
Closure: {
	Wren: {
		'mwasplund|soup-test-cpp': { Version: './', Build: '0', Tool: '0' }
		'soup-test-cpp': { Version: './', Build: '0', Tool: '0' }
		'soup|build-utils': { Version: 0.9.2, Digest: 'sha256:5350668c3d273aeeb1718ac1520ad24c24b0c8640785286bbca788e7efa7da3c', Build: '0', Tool: '0' }
		'soup|cpp-compiler': { Version: 0.14.2, Digest: 'sha256:443b9feed4040e881730f78f19980c8fff333486bffe4a3a0579fd0ec8c0a4b8', Build: '0', Tool: '0' }
		'soup|cpp-compiler-clang': { Version: 0.8.0, Digest: 'sha256:3789bd7b88bdab4deab3aee2ce8a5eff3dbc34b5282e93544e5345edff55c9b0', Build: '0', Tool: '0' }
		'soup|cpp-compiler-gcc': { Version: 0.7.2, Digest: 'sha256:a58af2b0c398db4ce983f0ed9c674ea05d8d9260bd49e8eb2f8209c194895688', Build: '0', Tool: '0' }
		'soup|cpp-compiler-msvc': { Version: 0.14.2, Digest: 'sha256:209594330499b4bc048d68f004b93aa2d6ef6e5f3962addf839f834e6ca32415', Build: '0', Tool: '0' }
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