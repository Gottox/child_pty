var pty = require('../build/Release/pty.node'),
	child_process = require('child_process'),
	extend = require('extend'),
	util = require('util'),
	fs = require('fs');

var EXEC_HELPER = __dirname+'/../build/Release/exechelper';

/**
* PtyReadStream is a wrapper around fs.ReadStream. It sets
* sane default options for the use with PTYs and adds a
* resize function.
*/
function PtyReadStream(term, options) {
	this.columns = term.columns;
	this.rows = term.rows;
	this.master_fd = term.master_fd;
	this.ttyname = term.ttyname;
	options = extend({}, options, {
		fd: this.master_fd,
		autoClose: false
	});
	PtyReadStream.super_.call(this, null, options);
}
util.inherits(PtyReadStream, fs.ReadStream);

/**
* resizes the underlying PTY to size.column x size.rows.
*/
PtyReadStream.prototype.resize = function(size) {
	pty.resize(this, size);
	this.emit('resize');
};

/**
* event handler that does a clean shutdown on the
* PTY for certian errors.
*/
function errIgnore(err) {
	if(err.code === 'EIO' || err.code === 'EBADF')
		this.destroy();
	else
		throw err;
}

module.exports.spawn = function(command, args, options) {
	// Argument Parsing
	if(!util.isArray(args)) {
		options = args;
		args = [];
	}

	// Opening the PTY.
	term = pty.open(options);

	options = extend({}, options, {
		/* stdin, stdout, stderr */
		stdio: [ term.slave_fd, term.slave_fd, term.slave_fd ],
		detached: true
	});
	// add a message channel to retrieve error codes from EXEC_HELPER
	args.unshift(options.stdio.push('pipe', 'pipe') - 2, command);

	child = child_process.spawn(EXEC_HELPER, args, options);

	// set up message receiver on the client
	var statePipes = child.stdio.splice(-2);
	statePipes[0].on('data', function(data) {
		var err = util._errnoException(- data.readInt32BE(0), 'spawn ' + command);
		err.path = command;
		err.spawnargs = args.slice(2);
		child.emit('error', err);
		child.removeAllListeners();
		statePipes[1].end();
	});
	fs.close(term.slave_fd);

	// The user interacts with the master fd, so overwrite those two values.
	child.stdin = fs.createWriteStream(null, { fd:term.master_fd, autoClose: false }).on('error', errIgnore);

	// Ignore certain errors on stdout.
	child.stdout = new PtyReadStream(term).on('error', errIgnore);
	child.on('exit', function() {
		this.stdin.close();
		this.stdout.close();
	});
	return child;
};
