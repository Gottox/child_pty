child_pty
=========

child_pty is a module for creating and interacting with pseudo terminals. It
tries to be as minimal as possible and borrows most of its functionality from
child_process.

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

See [terminal.js-webshell](http://github.com/Gottox/terminal.js-webshell)
