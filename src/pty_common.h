#include <nan.h>
NAN_METHOD(Resize);
NAN_METHOD(Open);

void Init(v8::Handle<v8::Object> exports) {
	Nan::SetMethod(exports, "open", Open);
	Nan::SetMethod(exports, "resize", Resize);
}

NODE_MODULE(pty, Init)
