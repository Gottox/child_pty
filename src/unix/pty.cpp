#include <errno.h>
#if   defined(__linux)
#	include <pty.h>
#	include <unistd.h>
#	include <termios.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#	include <sys/ioctl.h>
#	include <unistd.h>
/*
 * from pty.js:
 * From node v0.10.28 (at least?) there is also a "util.h" in node/src,
 * which would confuse the compiler when looking for "util.h".
 */
#	if NODE_VERSION_AT_LEAST(0, 10, 28)
#		include <../include/util.h>
#	else
#		include <util.h>
#	endif
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#	include <libutil.h>
#endif

NAN_METHOD(Resize) {
	Nan::HandleScope scope;
	struct winsize w;
	memset(&w, 0, sizeof w);
	v8tows(&w.ws_row, &w.ws_col, info[1]);
	v8::Handle<v8::Object> obj = info[0]->ToObject();
	if(ioctl(obj->Get(Nan::New<v8::String>("master_fd").ToLocalChecked())->Uint32Value(), TIOCSWINSZ, &w) < 0)
		return Nan::ThrowError(strerror(errno));
	wstov8(obj, w.ws_row, w.ws_col);
	return;
}

NAN_METHOD(Open) {
	Nan::HandleScope scope;
	struct winsize w;
	int master, slave;
	char *tty;
	memset(&w, 0, sizeof w);
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();
	v8tows(&w.ws_row, &w.ws_col, info[0]);
	if(openpty(&master, &slave, NULL, NULL, &w) < 0 ||
			(tty = ttyname(slave)) == NULL)
		return Nan::ThrowError(strerror(errno));
	obj->Set(Nan::New<v8::String>("ttyname").ToLocalChecked(), Nan::New<v8::String>(tty).ToLocalChecked());
	obj->Set(Nan::New<v8::String>("master_fd").ToLocalChecked(), Nan::New<v8::Integer>(master));
	obj->Set(Nan::New<v8::String>("slave_fd").ToLocalChecked(), Nan::New<v8::Integer>(slave));
	wstov8(obj, w.ws_row, w.ws_col);
	info.GetReturnValue().Set(obj);
}

