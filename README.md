[![Build Status](https://travis-ci.org/Gottox/child_pty.png)](https://travis-ci.org/Gottox/child_pty)

child_pty
=========

child_pty is a module for creating and interacting with pseudo terminals. It
tries to be as minimal as possible and borrows most of its functionality from
child_process.

child_pty was written as a replacement for [pty.js](https://github.com/chjj/pty.js/).

API
---

It's mostly the same API
[child_process](http://nodejs.org/api/child_process.html) with the following
differences:

* Only ```child_pty.spawn()``` is supported

* The ```options``` argument of child_pty.spawn() has the following changes:
  * new field: ```options.columns```: columns of the instanciated PTY.
  * new field: ```options.rows```: rows of the instanciated PTY.
  * ```options.detached``` and ```options.stdio``` are ignored.

* ChildProcess in child_pty vs ChildProcess in child_process:
  * ```#stderr``` is not available. Use ```#stdout``` instead.

  * There's ```#stdout.resize(size)``` to resize the underlying PTY.
    The size attribute should have the following fields:
    * ```size.columns```: columns of the instanciated PTY.
    * ```size.rows```: rows of the instanciated PTY.

  * There's a ``#stdout.ttyname`` property with the name of the tty (eg:
    ``/dev/ttys016``)

  * child processes won't get an EOF once you close stdin. You have to call
    ```ChildProcess#kill()``` explicitly.

Examples
--------

This example opens a PTY with ```/bin/sh```, resizes the terminal, executes
```ls -l```, and exits the shell.

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
* v0.3 - API changes to fit child_process
* v0.4 - remove deprecated APIs
* v0.5 - MacOS support
* v1.0 - Exposes TTY name to the API
