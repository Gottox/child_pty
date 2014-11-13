#if   defined(__linux)
 #include <pty.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
 #include <util.h>
#elif defined(__FreeBSD__) || defined(__DragonFly__)
 #include <libutil.h>
#endif

void
mkwinsize(struct winsize *w, Handle<Value> size) {
	w->ws_row = size->ToObject()->Get(NanNew<String>("rows"))->Uint32Value();
	w->ws_col = size->ToObject()->Get(NanNew<String>("colums"))->Uint32Value();
	w->ws_xpixel = 0;
	w->ws_ypixel = 0;
}

NAN_METHOD(Open) {
	struct winsize w;
	int master, slave;

	NanScope();

	mkwinsize(&w, args[0]);

	if(openpty(&master, &slave, NULL, NULL, &w) < 0)
		NanThrowError("openpty failed");

	Handle<Object> ret = NanNew<Object>();
	ret->Set(NanNew<String>("master"), NanNew<Integer>(master));
	ret->Set(NanNew<String>("slave"), NanNew<Integer>(slave));

	NanReturnValue(ret);
}

NAN_METHOD(Resize) {
	struct winsize w;

	NanScope();

	mkwinsize(&w, args[0]);

	if(ioctl(args.This()->Get(NanNew<String>("master"))->Uint32Value(), TIOCSWINSZ, &w) < 0)
		return NanThrowError("ioctl failed");

	NanReturnUndefined();
}
