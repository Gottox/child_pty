{
	'variables': {
		'conditions': [
			['OS=="win"', {
					'SRCPATH':'src/win',
				}, {
					'SRCPATH':'src/unix',
				}
			]
		]
	},
	'targets': [
		{
			'target_name': 'pty',
			'sources': [ '<(SRCPATH)/pty.cpp' ],
			'default_configuration': 'Release',
			'include_dirs' : [
				"<!(node -e \"require('nan')\")"
			],
			'conditions': [
				['OS=="win"', {
					'dependencies': [
						'<(SRCPATH)/winpty/src/winpty.gyp:winpty'
					]
				}, {
					'libraries': [
						'-lutil',
					]
				}]
			]
		}, {
			'target_name': 'exechelper',
			'type': 'executable',
			'sources': [ '<(SRCPATH)/exechelper.cpp', ],
			'conditions': [
				['OS=="win"', {
					'dependencies': [
						'<(SRCPATH)/winpty/src/winpty.gyp:winpty'
					],
				}]
			]
		}
	]
}
