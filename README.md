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

* The ```options``` argument of child_pty.spawn() supports a ```size``` field
  which must contain the following:
  * ```size.columns```: columns of the instanciated pty.
  * ```size.rows```: rows of the instanciated pty.
  
* ```ChildProcess#stderr``` is not available. Use ```ChildProcess#stdout```
  instead

* There's a ```ChildProcess#resize()``` function which will take a size-object
  described above.

* child processes won't get an EOF once you close stdin. You have to call
  ```ChildProcess#kill``` explicitly.

* ```ChildProcess``` emits a 'resize' event when the pty is resized.

Examples
--------

This example opens a pty with ```/bin/sh``` does an ls -l and
exits the shell.

```javascript
var child_pty = require('child_pty');
var child = child_pty.spawn('/bin/sh', []);
child.stdout.pipe(process.stdout);
child.stdin.write('ls -l\n');
child.stdin.write('exit\n');
```

Changelog
---------

* v0.1.0 - initial release
* v0.2.0 - fix job control for shells
