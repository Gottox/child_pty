#if   defined(__linux)
 #include <pty.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
 #include <util.h>
#elif defined(__FreeBSD__) || defined(__DragonFly__)
 #include <libutil.h>
#endif


NAN_METHOD(Open) {

	NanReturnUndefined();
}

NAN_METHOD(Resize) {

	NanReturnUndefined();
}
