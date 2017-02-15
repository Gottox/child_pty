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
makews(struct winsize *w, v8::Handle<v8::Value> size) {
	v8::Handle<v8::String> columns = Nan::New<v8::String>("columns").ToLocalChecked(),
		rows = Nan::New<v8::String>("rows").ToLocalChecked();
	w->ws_row = 24;
	w->ws_col = 80;
	w->ws_xpixel = w->ws_ypixel = 0;
	if(size->IsObject()) {
		v8::Handle<v8::Object> obj = size->ToObject();
		if(obj->Has(columns))
			w->ws_col = obj->Get(columns)->Uint32Value();
		if(obj->Has(rows))
			w->ws_row = obj->Get(rows)->Uint32Value();
	}
}

static void
applyws(struct winsize *w, v8::Handle<v8::Object> obj) {
	obj->Set(Nan::New<v8::String>("rows").ToLocalChecked(), Nan::New<v8::Integer>(w->ws_row));
	obj->Set(Nan::New<v8::String>("columns").ToLocalChecked(), Nan::New<v8::Integer>(w->ws_col));
}

NAN_METHOD(Resize) {
	struct winsize w;
	Nan::HandleScope scope;
	makews(&w, info[1]);
	v8::Handle<v8::Object> obj = info[0]->ToObject();
	if(ioctl(obj->Get(Nan::New<v8::String>("master_fd").ToLocalChecked())->Uint32Value(), TIOCSWINSZ, &w) < 0)
		return Nan::ThrowError(strerror(errno));
	applyws(&w, obj);
	return;
}

NAN_METHOD(Open) {
	struct winsize w;
	int master, slave;
	char *tty;
	Nan::HandleScope scope;
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();
	makews(&w, info[0]);
	if(openpty(&master, &slave, NULL, NULL, &w) < 0 ||
			(tty = ttyname(slave)) == NULL)
		return Nan::ThrowError(strerror(errno));
	obj->Set(Nan::New<v8::String>("ttyname").ToLocalChecked(), Nan::New<v8::String>(tty).ToLocalChecked());
	obj->Set(Nan::New<v8::String>("master_fd").ToLocalChecked(), Nan::New<v8::Integer>(master));
	obj->Set(Nan::New<v8::String>("slave_fd").ToLocalChecked(), Nan::New<v8::Integer>(slave));
	applyws(&w, obj);
	info.GetReturnValue().Set(obj);
}

void Init(v8::Handle<v8::Object> exports) {
	v8::Handle<v8::Object> modes;

	Nan::SetMethod(exports, "open", Open);
	Nan::SetMethod(exports, "resize", Resize);
}

NODE_MODULE(pty, Init)
