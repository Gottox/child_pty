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

#define EXPORT_SYMBOL(o, s) o->Set(Nan::New<v8::String>(#s).ToLocalChecked(), Nan::New<v8::Number>(s));

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

NAN_METHOD(Setattr) {
	struct termios tios;
	Nan::HandleScope scope;
	v8::Local<v8::Object> obj = info[2]->ToObject();
	tios.c_iflag = obj->Get(Nan::New<v8::String>("iflags").ToLocalChecked())->Uint32Value();
	tios.c_oflag = obj->Get(Nan::New<v8::String>("oflags").ToLocalChecked())->Uint32Value();
	tios.c_cflag = obj->Get(Nan::New<v8::String>("cflags").ToLocalChecked())->Uint32Value();
	tios.c_lflag = obj->Get(Nan::New<v8::String>("lflags").ToLocalChecked())->Uint32Value();
	if(tcsetattr(info[0]->ToObject()->Get(Nan::New<v8::String>("master_fd").ToLocalChecked())->Uint32Value(),
				info[1]->Uint32Value(), &tios) < 0)
		return Nan::ThrowError(strerror(errno));
	return;
}

NAN_METHOD(Getattr) {
	struct termios tios;
	Nan::HandleScope scope;
	if(tcgetattr(info[0]->ToObject()->Get(Nan::New<v8::String>("master_fd").ToLocalChecked())->Uint32Value(),
				&tios) < 0)
		return Nan::ThrowError(strerror(errno));
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();
	obj->Set(Nan::New<v8::String>("iflags").ToLocalChecked(), Nan::New<v8::Number>(tios.c_iflag));
	obj->Set(Nan::New<v8::String>("oflags").ToLocalChecked(), Nan::New<v8::Number>(tios.c_oflag));
	obj->Set(Nan::New<v8::String>("cflags").ToLocalChecked(), Nan::New<v8::Number>(tios.c_cflag));
	obj->Set(Nan::New<v8::String>("lflags").ToLocalChecked(), Nan::New<v8::Number>(tios.c_lflag));
	info.GetReturnValue().Set(obj);
}

void Init(v8::Handle<v8::Object> exports) {
	Nan::SetMethod(exports, "open", Open);
	Nan::SetMethod(exports, "resize", Resize);
	Nan::SetMethod(exports, "setattr", Setattr);
	Nan::SetMethod(exports, "getattr", Getattr);

	EXPORT_SYMBOL(exports, TCSANOW);
	EXPORT_SYMBOL(exports, TCSADRAIN);
	EXPORT_SYMBOL(exports, TCSAFLUSH);

	v8::Handle<v8::Object> modes = Nan::New<v8::Object>();

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
	//EXPORT_SYMBOL(modes, IUCLC);   /* translate upper case to lower case */

	/* Output Modes */
	EXPORT_SYMBOL(modes, OPOST);   /* enable following output processing */
	EXPORT_SYMBOL(modes, ONLCR);   /* map NL to CR-NL (ala CRMOD) */
	EXPORT_SYMBOL(modes, OCRNL);   /* map CR to NL */
	//EXPORT_SYMBOL(modes, OLCUC);   /* translate lower case to upper case */
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
	//EXPORT_SYMBOL(modes, XCASE);   /* canonical upper/lower case */

	exports->Set(Nan::New<v8::String>("modes").ToLocalChecked(), modes);
}

NODE_MODULE(pty, Init)
