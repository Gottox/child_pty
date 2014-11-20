var args = {};
for(var i = 0; i < process.argv.length; i++) {
	args[process.argv[i]] = true;
}

if(args.sigwinch)
	process.stdout.on('resize', function() {
		process.stdout.write('sigwinch '+
			process.stdout.columns+
			' '+process.stdout.rows+'\n');
	});

if(args.print)
	process.stdout.write('print\n');

if(args.echo)
	process.stdin.on('data', function(data) {
		process.stdout.write(data);
		process.exit();
	});

if(args.error)
	process.exit(4);
