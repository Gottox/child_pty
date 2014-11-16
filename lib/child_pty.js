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

module.exports.spawn = function() {
	var args = Array.prototype.slice.call(arguments),
		size = { cols: 80, rows: 24 },
		optind = args.length >= 3 || util.isArray(args[1]) ? 2 : 1,
		term;
	if(args[optind])
		extend(size, args[optind].size);
	term = pty.open(size);
	args[optind] = extend({}, args[optind], {
		stdio: [ term.slave, term.slave, term.slave ],
		detached: true
	});
	child = child_process.spawn.apply(child_process, args);
	fs.close(term.slave);
	child.stdin = fs.createWriteStream(null, { fd:term.master, autoClose: false }).on('error', errIgnore);
	child.stdout = fs.createReadStream(null,  { fd:term.master, autoClose: false }).on('error', errIgnore);
	child.resize = function(size) {
		term.resize(size);
		child.kill('SIGWINCH');
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
