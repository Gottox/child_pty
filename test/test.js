var assert = require('assert');
var child_pty = require('../index');

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

		child.stdout.on('data', function(data) {
			d += data.toString();
		});

		child.on('exit', function() {
			assert.equal('print\r\n', d, 'should receive data');
			done();
		});
	});

	it('should read from stdin', function(done) {
		spawn_client('echo');
		var d;

		child.stdout.on('data', function(data) {
			d = data;
			child.kill();
		});

		child.on('exit', function() {
			assert.equal('data\r\n', d, 'received data should be ok');
			done();
		});

		child.stdin.write('data\n');
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

		child.stdout.once('data', function(data) {
			assert.equal('print', data.toString().trim());
			child.stdout.resize({ columns:80, rows: 80 });
			child.stdout.once('data', function(data) {
				assert.equal('sigwinch '+this.columns+' '+this.rows+'\r\n', data.toString());
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
			child.stdin.write('Foobar');
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

	afterEach(function(){
		child.kill();
	});
});
