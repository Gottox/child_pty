var pty = require('../build/Release/pty.node'),
	child_process = require('child_process'),
	extend = require('extend'),
	util = require('util'),
	net = require('net'),
	events = require('events'),
	path = require('path'),
	fs = require('fs');
	termios = require('termios');
var _Handle = process.binding('pipe_wrap').Pipe;

var EXEC_HELPER = path.join(__dirname, '..', 'build', 'Release', 'exechelper');
var children = {};

// Cleans up processes when user sends SIGINT
function cleanup() {
	var keys = Object.keys(children);
	for(var i = 0; i < keys.length; i++) {
		if(!children[keys[i]].killed)
			children[keys[i]].kill('SIGHUP');
	}
}
process
	.once('SIGINT', cleanup)
	.once('exit', cleanup);
/**
* PtyRwStream is a wrapper around net.Socket.
*/
function PtyRwStream(term) {
	this.columns = term.columns;
	this.rows = term.rows;
	this.master_fd = term.master_fd;
	this.ttyname = term.ttyname;
	var handle = new _Handle();
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
	var i, ptyStream, ptyFds = [];
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

	for(i = 0; i < 3 || i < options.stdio; i++) {
		if((i < 3 && (options.stdio[i] === undefined || options.stdio[i] === null)) ||
				options.stdio[i] === 'pty') {
			options.stdio[i] = term.slave_fd;
			ptyFds.push(i);
		}
	}

	// add a message channel to retrieve error codes from EXEC_HELPER
	args.unshift(options.stdio.push('pipe') - 1, command);

	child = child_process.spawn(EXEC_HELPER, args, options);
	child.pty = ptyStream;
	fs.close(term.slave_fd);

	// Save child for killing it on SIGINT.
	children[child.pid] = child;

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
		delete children[child.pid];
	});

	child.once('exit', function() {
		ptyStream.emit('end');
		ptyStream.end();
		delete children[child.pid];
	});

	return child;
};
