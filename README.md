[![Build Status](https://travis-ci.org/Gottox/child_pty.png)](https://travis-ci.org/Gottox/child_pty)

child\_pty
=========

child\_pty is a module for creating and interacting with pseudo terminals. It
tries to be as minimal as possible and borrows most of its functionality from
child\_process.

API
---

See [child\_process](http://nodejs.org/api/child_process.html) with the following
changes:

* Only `child_pty.spawn()` is supported

### child\_pty.spawn()

* options fields:
  * new field: `options.columns`: columns of the instanciated PTY.
  * new field: `options.rows`: rows of the instanciated PTY.
  * `options.detached` is ignored.
  * `options.stdio` allows `'pty'` as array element. The value `'pty'`
    indicates, that this fd is bound to the pty.
  * `options.stdio` will default to `[ 'pty', 'pty', 'pty' ]`

### ChildProcess

ChildProcess of child\_pty uses the same prototype as child\_process. Its
instances differentiate in the following: 

* new field: `#pty` points to a PtyRwStream that's associated
  with the child processes pty.
* All file descriptors are bound to the pty in the `#stdio` array point to the
  the same object as `#pty`. This is also true for `#stdin` and
  `#stdout`.
* If stderr is bound to the pty the field `#stderr` will be a dummy Event
  Emitter that will never emit any events.

### PtyRwStream

PtyRwStream is a net.Socket with the following changes
* `#resize(size)`: resizes the underlying pty.
  The size attribute should have the following fields:
  * `#size.columns`: columns of the instanciated PTY.
  * `#size.rows`: rows of the instanciated PTY.
* `#ttyname`: property with the name of the tty (eg:
  `/dev/ttys016`)
* `setattr(attr)`: sets terminal attributes. `attr` may be an object containing
  one or more of the following fields:
  * `iflag`: input modes
  * `oflag`: output modes
  * `cflag`: control modes
  * `lflag`: local modes
  for further explainations on the allowed values consider the `termios(3)`
  manpage.
* `getattr(attr)`: sets terminal attributes. `attr` may be an object containing
  one or more of the following fields:
  * `iflag`: input modes
  * `oflag`: output modes
  * `cflag`: control modes
  * `lflag`: local modes
  for further explainations on the allowed values consider the `termios(3)`
  manpage.
* due to the nature of PTYs it's neither possible to get 'end' events from
  the underlying process when it closes its pty file descriptors nor will call
  `#end()` close the child processes file descriptor. To end the underlying
  process call `ChildProcess#kill('SIGHUP')` instead.
* PtyRwStream will emit the `'end'` Event when the child process exits. It
  will *not* emit the `'end'` event if the child process closes its slave file
  descriptor. See above.

Examples
--------

This example opens a PTY with `/bin/sh`, resizes the terminal, executes
`ls -l`, and exits the shell.

```javascript
var child_pty = require('child_pty');
var child = child_pty.spawn('/bin/sh', []);
child.stdout.on('resize', function() {
	console.log('New columns: ' + this.columns);
	console.log('New rows:    ' + this.rows);
}).pipe(process.stdout);
child.stdout.resize({ columns: 80, rows: 48 });
child.stdin.write('ls -l\n');
child.stdin.write('exit\n');
```

Changelog
---------

* v0.1 - initial release
* v0.2 - fix job control for shells
* v0.3 - API changes to fit child\_process
* v0.4 - remove deprecated APIs
* v0.5 - MacOS support
* v1.0 - Exposes TTY name to the API
* v1.1 - Exposes tcgetattr/tcsetattr functions; node-4.0 support
* v2.0 - child\_pty now emits the error event when a child can't be
  spawned instead of printing an error to stdout.
* v3.0 - child\_pty IO handling has been rewritten.
  the end event is emitted on the pty when the child program exits.
  all fds opened on the pty point to the same Stream instance.
