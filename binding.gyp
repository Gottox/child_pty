{
	'targets': [
		{
			'target_name': 'pty',
			"sources": [ "src/pty.cpp" ],
			'default_configuration': 'Release',
			"include_dirs" : [
				"<!(node -e \"require('nan')\")"
			],
			'libraries': [
				'-lutil',
			],
		},{
			'target_name': 'exechelper',
			'type': 'executable',
			'conditions': [
				['OS=="win"', {
					'sources': [
						'src/exechelper_win.cpp',
					],
					'dependencies': [
						'src/winpty/src/winpty.gyp:winpty',
						'src/winpty/src/winpty.gyp:winpty-agent'
					],
				}, {
					'sources': [
						'src/exechelper_unix.cpp',
					],
				}]
			]
		}
	]
}
