{
	'targets': [{
		'target_name': 'pty',
		"sources": [ "src/pty.cpp" ],
		'default_configuration': 'Release',
		"include_dirs" : [
			"<!(node -e \"require('nan')\")"
		],
		'conditions': [
			['OS=="win"', {
				'dependencies' : [
					'3rd/winpty/winpty.gyp:winpty-agent',
					'3rd/winpty/winpty.gyp:winpty'
				],
				'libraries': [
					'shlwapi.lib'
				]}, {
					'libraries': [
						'-lutil',
					]
				}
			]
		]
	},{
		'target_name': 'exechelper',
		'type': 'executable',
		'sources': [
			'src/exechelper.cpp',
		]
	}]
}
