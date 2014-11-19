var pty = require('../build/Release/pty.node'),
	child_process = require('child_process'),
	extend = require('extend'),
	util = require('util'),
	fs = require('fs');

function errIgnore(err) {
	if(err.code === 'EIO' || err.code === 'EBADF')
		this.destroy();
	else
		throw err;
}

module.exports.spawn = function(command, args, options) {
	var size = { cols: 80, rows: 24 };
	if(!util.isArray(args)) {
		options = args;
		args = [];
	}
	if(options)
		extend(size, options.size);
	term = pty.open(size);
	options = extend({}, options, {
		stdio: [ term.slave, term.slave, term.slave ],
		detached: true
	});
	args.unshift(command);
	child = child_process.spawn(__dirname+'/../build/Release/exechelper', args, options);
	fs.close(term.slave);
	child.stdin = fs.createWriteStream(null, { fd:term.master, autoClose: false }).on('error', errIgnore);
	child.stdout = fs.createReadStream(null,  { fd:term.master, autoClose: false }).on('error', errIgnore);
	child.resize = function(size) {
		term.resize(size);
		this.columns = size.columns || this.stdout.columns;
		this.rows = size.rows || this.stdout.rows;
		this.emit('resize');
	};
	child.on('exit', function() {
		this.stdin.close();
		this.stdout.close();
	});
	child.columns = size.columns;
	child.rows = size.rows;
	return child;
};
