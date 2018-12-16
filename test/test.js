var assert = require('assert');
var child_pty = require('../index');
var readline = require('readline');
var stream = require('stream');

function write_cb(err) {
	if(err) throw err;
}
function lb(s) {
	return readline.createInterface({
		input: s,
		output: new stream.Writable()
	});
}

describe('child_pty.spawn()', function(){
	var child;
	function spawn_client() {
		child = child_pty.spawn(process.argv[0],
			Array.prototype.concat.apply([ __dirname + '/libexec/client.js'],
			   arguments));
	}

	it('should exit', function(done){
		spawn_client();

		child.on('exit', function() {
			done();
		});
	});

	it('should print to stdout', function(done){
		spawn_client('print');
		var d = '';

		lb(child.stdout).on('line', function(data) {
			d += data.toString();
		});

		child.on('exit', function() {
			assert.equal('print', d, 'should receive data');
			done();
		});
	});

	it('should read from stdin', function(done) {
		spawn_client('echo');
		var d;

		lb(child.stdout).on('line', function(data) {
			d = data;
			child.kill();
		});

		child.on('exit', function() {
			assert.equal('data', d, 'received data should be ok');
			done();
		});

		child.stdin.write('data\n', write_cb);
	});

	it('should fire event on resize', function(done) {
		spawn_client();
		var resized = false;

		child.stdout.on('resize', function() {
			resized = true;
		});
		child.on('exit', function() {
			assert.ok(resized, 'resize should be emitted');
			done();
		});

		child.stdout.resize({columns: 80, rows: 80});
	});

	it('should send sigwinch on resize', function(done) {
		spawn_client('print', 'echo', 'sigwinch');

		lb(child.stdout).once('line', function(data) {
			assert.equal('print', data.toString());
			child.stdout.resize({ columns:80, rows: 80 });
			lb(child.stdout).once('line', function(data) {
				assert.equal('sigwinch '+child.stdout.columns+' '+child.stdout.rows, data.toString());
				child.kill();
			});
		});
		child.on('exit', function() {
			done();
		});
	});

	it('should have ttyname', function() {
		spawn_client();

		assert.equal(typeof child.stdout.ttyname, 'string');
	});

	it('should allow to open multiple terminals', function(done) {
		var completed = 0, parallel = 100;
		function complete() {
			completed++;
			if(completed === parallel)
				done();
		}

		for(var i = 0; i < parallel; i++) {
			var child = child_pty.spawn('sleep', [ '1' ]);
			child.stdin.write('Foobar', write_cb);
			child.on('exit', complete);
		}
	});

	it('should emit "error" if executable doesn\'t exist', function(done) {
		var nonexistent = 'This Does Not Exist';
		child_pty.spawn(nonexistent, [1]).on('error', function(err) {
			assert.ok(typeof err === 'object');
			done();
		});
	});

	it('should close the pty when the child exits', function(done) {
		var child = child_pty.spawn('/bin/echo', ['200']);
		child.pty.on('end', function() {
			done();
		});
	});

	it('does not deadlock if piped in a row', function(done) {
		var completed = 0, parallel = 20, childs = [];
		function complete() {
			completed++;
			if(completed === parallel)
				done();
		}

		for(var i = 0, child; i < parallel; i++) {
			child = child_pty.spawn('/bin/cat');
			child.on('exit', complete);
			childs.push(child);
			if(i)
				childs[i-1].stdout.pipe(child.stdin);
		}
		childs[0].stdin.write('test\n', write_cb);
		childs[childs.length-1].stdout.on('data', function() {
			for(var i = 0; i < parallel; i++) {
				childs[i].kill();
			}
		});
	});

	it('closes the underlying process on exit', function(done) {
		var child = child_pty.spawn('/bin/cat', '-');
		child.pty.destroy();
		child.on('exit', done);
	});

	afterEach(function(){
		child.kill();
	});
});
