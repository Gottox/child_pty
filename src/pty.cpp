#include <nan.h>
#include <errno.h>
#if   defined(__linux)
#	include <pty.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#	include <util.h>
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#	include <libutil.h>
#endif

void mkwinsize(struct winsize *w, v8::Handle<v8::Value> size) {
	w->ws_row = size->ToObject()->Get(NanNew<v8::String>("rows"))->Uint32Value();
	w->ws_col = size->ToObject()->Get(NanNew<v8::String>("colums"))->Uint32Value();
	w->ws_xpixel = 0;
	w->ws_ypixel = 0;
}

NAN_METHOD(Resize) {
	struct winsize w;
	NanScope();
	mkwinsize(&w, args[0]);
	if(ioctl(args.This()->Get(NanNew<v8::String>("master"))->Uint32Value(), TIOCSWINSZ, &w) < 0)
		return NanThrowError("ioctl failed");
	NanReturnUndefined();
}

NAN_METHOD(Open) {
	struct winsize w;
	int master, slave;
	NanScope();
	mkwinsize(&w, args[0]);
	if(openpty(&master, &slave, NULL, NULL, &w) < 0)
		NanThrowError(strerror(errno));
	v8::Handle<v8::Object> ret = NanNew<v8::Object>();
	ret->Set(NanNew<v8::String>("master"), NanNew<v8::Integer>(master));
	ret->Set(NanNew<v8::String>("slave"), NanNew<v8::Integer>(slave));
	NODE_SET_METHOD(ret, "resize", Resize);
	NanReturnValue(ret);
}

void Init(v8::Handle<v8::Object> exports) {
	NODE_SET_METHOD(exports, "open", Open);
}

NODE_MODULE(pty, Init)
