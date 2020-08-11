#include <nan.h>
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

static void
v8tows(struct winsize *w, v8::Local<v8::Value> size) {
	v8::Local<v8::String> columns = Nan::New<v8::String>("columns").ToLocalChecked(),
		rows = Nan::New<v8::String>("rows").ToLocalChecked();
	w->ws_row = 24;
	w->ws_col = 80;
	w->ws_xpixel = w->ws_ypixel = 0;
	if(size->IsObject()) {
		v8::Local<v8::Object> obj = Nan::To<v8::Object>(size).ToLocalChecked();
		if(!Nan::Has(obj, columns).IsNothing())
			w->ws_col = Nan::Get(obj, columns).ToLocalChecked()->ToUint32(Nan::GetCurrentContext()).ToLocalChecked()->Value();
		if(!Nan::Has(obj, rows).IsNothing())
			w->ws_row = Nan::Get(obj, rows).ToLocalChecked()->ToUint32(Nan::GetCurrentContext()).ToLocalChecked()->Value();
	}
}

static void
wstov8(v8::Local<v8::Object> obj, struct winsize *w) {
	Nan::Set(obj, Nan::New<v8::String>("rows").ToLocalChecked(), Nan::New<v8::Integer>(w->ws_row));
	Nan::Set(obj, Nan::New<v8::String>("columns").ToLocalChecked(), Nan::New<v8::Integer>(w->ws_col));
}

NAN_METHOD(Resize) {
	Nan::HandleScope scope;
	struct winsize w;
	v8tows(&w, info[1]);
	v8::Local<v8::Object> obj = Nan::To<v8::Object>(info[0]).ToLocalChecked();
	if(ioctl(Nan::Get(obj, Nan::New<v8::String>("master_fd").ToLocalChecked()).ToLocalChecked()->ToUint32(Nan::GetCurrentContext()).ToLocalChecked()->Value(), TIOCSWINSZ, &w) < 0)
		return Nan::ThrowError(strerror(errno));
	wstov8(obj, &w);
	return;
}

NAN_METHOD(Open) {
	struct winsize w;
	int master, slave;
	char *tty;
	Nan::HandleScope scope;
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();
	v8tows(&w, info[0]);
	if(openpty(&master, &slave, NULL, NULL, &w) < 0 ||
			(tty = ttyname(slave)) == NULL)
		return Nan::ThrowError(strerror(errno));
	Nan::Set(obj, Nan::New<v8::String>("ttyname").ToLocalChecked(), Nan::New<v8::String>(tty).ToLocalChecked());
	Nan::Set(obj, Nan::New<v8::String>("master_fd").ToLocalChecked(), Nan::New<v8::Integer>(master));
	Nan::Set(obj, Nan::New<v8::String>("slave_fd").ToLocalChecked(), Nan::New<v8::Integer>(slave));
	wstov8(obj, &w);
	info.GetReturnValue().Set(obj);
}

void Init(v8::Local<v8::Object> exports) {
	v8::Local<v8::Object> modes;

	Nan::SetMethod(exports, "open", Open);
	Nan::SetMethod(exports, "resize", Resize);
}

NODE_MODULE(pty, Init)
