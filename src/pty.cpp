#include <nan.h>
#include <errno.h>
#if   defined(__linux)
#	include <pty.h>
#	include <unistd.h>
#	include <termios.h>
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

#define EXPORT_SYMBOL(o, s) o->Set(NanNew<v8::String>(#s), NanNew<v8::Integer>(s));

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

NAN_METHOD(Setattr) {
	struct termios tios;
	NanScope();
	v8::Handle<v8::Object> obj = args[2]->ToObject();
	tios.c_iflag = obj->Get(NanNew<v8::String>("iflags"))->Uint32Value();
	tios.c_oflag = obj->Get(NanNew<v8::String>("oflags"))->Uint32Value();
	tios.c_cflag = obj->Get(NanNew<v8::String>("cflags"))->Uint32Value();
	tios.c_lflag = obj->Get(NanNew<v8::String>("lflags"))->Uint32Value();
	if(tcsetattr(args[0]->ToObject()->Get(NanNew<v8::String>("master_fd"))->Uint32Value(),
				args[1]->Uint32Value(), &tios) < 0)
		return NanThrowError(strerror(errno));
	NanReturnUndefined();
}

NAN_METHOD(Getattr) {
	struct termios tios;
	NanScope();
	if(tcgetattr(args[0]->ToObject()->Get(NanNew<v8::String>("master_fd"))->Uint32Value(),
				&tios) < 0)
		return NanThrowError(strerror(errno));
	v8::Handle<v8::Object> obj = NanNew<v8::Object>();
	obj->Set(NanNew<v8::String>("iflags"), NanNew<v8::Integer>(tios.c_iflag));
	obj->Set(NanNew<v8::String>("oflags"), NanNew<v8::Integer>(tios.c_oflag));
	obj->Set(NanNew<v8::String>("cflags"), NanNew<v8::Integer>(tios.c_cflag));
	obj->Set(NanNew<v8::String>("lflags"), NanNew<v8::Integer>(tios.c_lflag));
	NanReturnValue(obj);
}

void Init(v8::Handle<v8::Object> exports) {
	NODE_SET_METHOD(exports, "open", Open);
	NODE_SET_METHOD(exports, "resize", Resize);
	NODE_SET_METHOD(exports, "setattr", Setattr);
	NODE_SET_METHOD(exports, "getattr", Getattr);

	EXPORT_SYMBOL(exports, TCSANOW);
	EXPORT_SYMBOL(exports, TCSADRAIN);
	EXPORT_SYMBOL(exports, TCSAFLUSH);

	v8::Handle<v8::Object> modes = NanNew<v8::Object>();

	/* Input Modes */
	EXPORT_SYMBOL(modes, IGNBRK);  /* ignore BREAK condition */
	EXPORT_SYMBOL(modes, BRKINT);  /* map BREAK to SIGINT */
	EXPORT_SYMBOL(modes, IGNPAR);  /* ignore (discard) parity errors */
	EXPORT_SYMBOL(modes, PARMRK);  /* mark parity and framing errors */
	EXPORT_SYMBOL(modes, INPCK);   /* enable checking of parity errors */
	EXPORT_SYMBOL(modes, ISTRIP);  /* strip 8th bit off chars */
	EXPORT_SYMBOL(modes, INLCR);   /* map NL into CR */
	EXPORT_SYMBOL(modes, IGNCR);   /* ignore CR */
	EXPORT_SYMBOL(modes, ICRNL);   /* map CR to NL (ala CRMOD) */
	EXPORT_SYMBOL(modes, IXON);    /* enable output flow control */
	EXPORT_SYMBOL(modes, IXOFF);   /* enable input flow control */
	EXPORT_SYMBOL(modes, IXANY);   /* any char will restart after stop */
	EXPORT_SYMBOL(modes, IMAXBEL); /* ring bell on input queue full */
	EXPORT_SYMBOL(modes, IUCLC);   /* translate upper case to lower case */

	/* Output Modes */
	EXPORT_SYMBOL(modes, OPOST);   /* enable following output processing */
	EXPORT_SYMBOL(modes, ONLCR);   /* map NL to CR-NL (ala CRMOD) */
	EXPORT_SYMBOL(modes, OCRNL);   /* map CR to NL */
	EXPORT_SYMBOL(modes, OLCUC);   /* translate lower case to upper case */
	EXPORT_SYMBOL(modes, ONOCR);   /* No CR output at column 0 */
	EXPORT_SYMBOL(modes, ONLRET);  /* NL performs the CR function */

	/* Control Modes */
	EXPORT_SYMBOL(modes, CSIZE);   /* character size mask */
	EXPORT_SYMBOL(modes, CS5);     /* 5 bits (pseudo) */
	EXPORT_SYMBOL(modes, CS6);     /* 6 bits */
	EXPORT_SYMBOL(modes, CS7);     /* 7 bits */
	EXPORT_SYMBOL(modes, CS8);     /* 8 bits */
	EXPORT_SYMBOL(modes, CSTOPB);  /* send 2 stop bits */
	EXPORT_SYMBOL(modes, CREAD);   /* enable receiver */
	EXPORT_SYMBOL(modes, PARENB);  /* parity enable */
	EXPORT_SYMBOL(modes, PARODD);  /* odd parity, else even */
	EXPORT_SYMBOL(modes, HUPCL);   /* hang up on last close */
	EXPORT_SYMBOL(modes, CLOCAL);  /* ignore modem status lines */

	/* Local Modes */
	EXPORT_SYMBOL(modes, ECHOKE);  /* visual erase for line kill */
	EXPORT_SYMBOL(modes, ECHOE);   /* visually erase chars */
	EXPORT_SYMBOL(modes, ECHOK);   /* echo NL after line kill */
	EXPORT_SYMBOL(modes, ECHO);    /* enable echoing */
	EXPORT_SYMBOL(modes, ECHONL);  /* echo NL even if ECHO is off */
	EXPORT_SYMBOL(modes, ECHOPRT); /* visual erase mode for hardcopy */
	EXPORT_SYMBOL(modes, ECHOCTL); /* echo control chars as ^(Char) */
	EXPORT_SYMBOL(modes, ISIG);    /* enable signals INTR, QUIT, [D]SUSP */
	EXPORT_SYMBOL(modes, ICANON);  /* canonicalize input lines */
	EXPORT_SYMBOL(modes, IEXTEN);  /* enable DISCARD and LNEXT */
	EXPORT_SYMBOL(modes, EXTPROC); /* external processing */
	EXPORT_SYMBOL(modes, TOSTOP);  /* stop background jobs from output */
	EXPORT_SYMBOL(modes, FLUSHO);  /* output being flushed (state) */
	EXPORT_SYMBOL(modes, PENDIN);  /* XXX retype pending input (state) */
	EXPORT_SYMBOL(modes, NOFLSH);  /* don't flush after interrupt */
	EXPORT_SYMBOL(modes, XCASE);   /* canonical upper/lower case */

	exports->Set(NanNew<v8::String>("modes"), modes);
}

NODE_MODULE(pty, Init)
