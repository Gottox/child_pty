var pty = require('../build/Release/pty.node'),
	child_process = require('child_process'),
	extend = require('extend'),
	util = require('util'),
	net = require('net'),
	events = require('events'),
	path = require('path'),
	fs = require('fs'),
	termios = require('termios');
var _handle = process.binding('pipe_wrap');

var EXEC_HELPER = path.join(__dirname, '..', 'build', 'Release', 'exechelper');

/**
* PtyRwStream is a wrapper around net.Socket.
*/
function PtyRwStream(term) {
	this.columns = term.columns;
	this.rows = term.rows;
	this.master_fd = term.master_fd;
	this.ttyname = term.ttyname;
	var handle = new _handle.Pipe((_handle.constants || {}).SOCKET);
	handle.open(term.master_fd);
	PtyRwStream.super_.call(this, {
		handle: handle,
		readable: true,
		writeable: true
	});
}
util.inherits(PtyRwStream, net.Socket);
module.exports.PtyRwStream = PtyRwStream;

PtyRwStream.prototype._destroy = function(ex, cb) {
	if(ex && ex.code === 'EIO')
		ex = null;
	PtyRwStream.super_.prototype._destroy.call(this, ex, cb);
};

/**
* sets the parameters associated with the terminal
*/
PtyRwStream.prototype.setattr = function(attr) {
	return termios.setattr(this.master_fd, attr);
};

/**
* gets the parameters associated with the terminal
*/
PtyRwStream.prototype.getattr = function() {
	return termios.getattr(this.master_fd);
};

/**
* resizes the underlying PTY to size.column x size.rows.
*/
PtyRwStream.prototype.resize = function(size) {
	pty.resize(this, size);
	this.emit('resize');
};

module.exports.spawn = function(command, args, options) {
	var i, ptyStream, ptyFds = [], client_slave;
	// Argument Parsing
	if(!util.isArray(args)) {
		options = args;
		args = [];
	}

	options = extend(true, { stdio: [] }, options, {
		detached: true
	});

	// Opening and configuring the PTY.
	term = pty.open(options);
	termios.setattr(term.master_fd, options);
	ptyStream = new PtyRwStream(term);
	client_slave = -1;

	for(i = 0; i < 3 || i < options.stdio.length; i++) {
		if((i < 3 && (options.stdio[i] === undefined || options.stdio[i] === null)) ||
				options.stdio[i] === 'pty') {
			if(client_slave === -1)
				client_slave = i;
			options.stdio[i] = term.slave_fd;
			ptyFds.push(i);
		}
	}

	// add a message channel to retrieve error codes from EXEC_HELPER
	// also add file descriptor of the master pty, so the child can close it
	args.unshift(options.stdio.push('pipe') - 1, term.master_fd, client_slave, command);

	child = child_process.spawn(EXEC_HELPER, args, options);
	child.pty = ptyStream;
	fs.close(term.slave_fd);

	for(i = 0; i < ptyFds.length; i++) {
		child.stdio[ptyFds[i]] = ptyStream;
		switch(ptyFds[i]) {
		case 0: child.stdin = ptyStream; break;
		case 1: child.stdout = ptyStream; break;
		// Dummy Emitter to be compatible to child_process
		case 2: child.stderr = new events.EventEmitter(); break;
		}
	}

	// set up message receiver on the client
	child.stdio.pop().once('data', function(data) {
		var err, errno = - data.readInt32BE(0);
		if(util._errnoException)
			err = util._errnoException(errno, 'spawn ' + command);
		else
			err = new Error('spawn ' + command + ' ' + errno);
		err.path = command;
		err.spawnargs = args.slice(2);
		child.emit('error', err);
		child.removeAllListeners();
		child.kill();
	});

	child.once('exit', function() {
		ptyStream.emit('end');
		ptyStream.end();
	});

	return child;
};
