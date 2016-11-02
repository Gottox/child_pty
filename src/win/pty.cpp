#include "../pty_common.h"

NAN_METHOD(Resize) {
	Nan::HandleScope scope;
	struct ws w = { .magic = MAGIC };
	v8tows(&w.r, &w.c, info[1]);
	v8::Handle<v8::Object> obj = info[0]->ToObject();

	write(obj->Get(Nan::New<v8::String>("master_fd").ToLocalChecked())->Uint32Value(),
			w, sizeof w);
}

NAN_METHOD(Open) {
	Nan::HandleScope scope;
	struct ws w = { .magic = MAGIC };
	v8tows(&w.r, &w.c, info[0]);
}
