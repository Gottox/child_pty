#include <nan.h>
#include <errno.h>
#if   defined(__linux)
#	include <pty.h>
#	include <unistd.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#	include <sys/ioctl.h>
/*
 * from pty.js:
 * From node v0.10.28 (at least?) there is also a "util.h" in node/src,
 * which would confuse the compiler when looking for "util.h".
 */
#	if NODE_VERSION_AT_LEAST(0, 10, 28)
#		include </usr/include/util.h>
#	else
#		include <util.h>
#	endif
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#	include <libutil.h>
#endif

static void
makews(struct winsize *w, v8::Handle<v8::Value> size) {
	v8::Handle<v8::String> columns = NanNew<v8::String>("columns"),
		rows = NanNew<v8::String>("rows");
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
	obj->Set(NanNew<v8::String>("rows"), NanNew<v8::Integer>(w->ws_row));
	obj->Set(NanNew<v8::String>("columns"), NanNew<v8::Integer>(w->ws_col));
}

NAN_METHOD(Resize) {
	struct winsize w;
	NanScope();
	makews(&w, args[1]);
	v8::Handle<v8::Object> obj = args[0]->ToObject();
	if(ioctl(obj->Get(NanNew<v8::String>("master_fd"))->Uint32Value(), TIOCSWINSZ, &w) < 0)
		return NanThrowError(strerror(errno));
	applyws(&w, obj);
	NanReturnUndefined();
}

NAN_METHOD(Open) {
	struct winsize w;
	int master, slave;
	char *tty;
	NanScope();
	v8::Handle<v8::Object> obj = NanNew<v8::Object>();
	makews(&w, args[0]);
	if(openpty(&master, &slave, NULL, NULL, &w) < 0 ||
			(tty = ttyname(slave)) == NULL)
		return NanThrowError(strerror(errno));
	obj->Set(NanNew<v8::String>("ttyname"), NanNew<v8::String>(tty));
	obj->Set(NanNew<v8::String>("master_fd"), NanNew<v8::Integer>(master));
	obj->Set(NanNew<v8::String>("slave_fd"), NanNew<v8::Integer>(slave));
	applyws(&w, obj);
	NanReturnValue(obj);
}

void Init(v8::Handle<v8::Object> exports) {
	NODE_SET_METHOD(exports, "open", Open);
	NODE_SET_METHOD(exports, "resize", Resize);
}

NODE_MODULE(pty, Init)
