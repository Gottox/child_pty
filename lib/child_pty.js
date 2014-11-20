var pty = require('../build/Release/pty.node'),
	child_process = require('child_process'),
	extend = require('extend'),
	util = require('util'),
	fs = require('fs');

var EXEC_HELPER = __dirname+'/../build/Release/exechelper';

function PtyReadStream(term, options) {
	this.columns = term.columns;
	this.rows = term.rows;
	this.master_fd = term.master_fd;
	options = extend({}, options, {
		fd: this.master_fd,
		autoClose: false
	});
	PtyReadStream.super_.call(this, null, options);
}
util.inherits(PtyReadStream, fs.ReadStream);

PtyReadStream.prototype.resize = function(size) {
	pty.resize(this, size);
	this.emit('resize');
};

function errIgnore(err) {
	if(err.code === 'EIO' || err.code === 'EBADF')
		this.destroy();
	else
		throw err;
}

module.exports.spawn = function(command, args, options) {
	if(!util.isArray(args)) {
		options = args;
		args = [];
	}
	// DEPRECATED: options.size
	if(options && options.size) {
		console.log('options.size is deprecated. '+
			'Use options.columns and options.rows instead.');
		options.rows = options.size.rows;
		options.columns = options.size.columns;
	}
	term = pty.open(options);
	options = extend({}, options, {
		stdio: [ term.slave_fd, term.slave_fd, term.slave_fd ],
		detached: true
	});
	args.unshift(command);
	child = child_process.spawn(EXEC_HELPER, args, options);
	fs.close(term.slave_fd);
	child.stdin = fs.createWriteStream(null, { fd:term.master_fd, autoClose: false }).on('error', errIgnore);
	child.stdout = new PtyReadStream(term).on('error', errIgnore);
	child.on('exit', function() {
		this.stdin.close();
		this.stdout.close();
	});

	// DEPRECATED: warn about the use of 'resize' on ChildProcess
	child.columns = options.columns;
	child.rows = options.rows;
	child.resize = function(size) {
		console.log('ChildProcess#resize() is deprecated. '+
			'Use child.stdout.resize() instead.');
		this.resize = this.stdout.resize.bind(this.stdout);
		return this.stdout.resize(size);
	};
	child.on('newListener', function newListenerCb(ev) {
		if(ev !== 'resize')
			return;
		console.log('\'resize\' on ChildProcess is deprecated. '+
			'Use child.stdout.on(\'resize\', ...) instead.');
		this.removeListener(ev, newListenerCb);
		this.stdout.on(ev, function() {
			child.columns = this.columns;
			child.rows = this.rows;
			child.emit(ev);
		});
	});
	return child;
};
