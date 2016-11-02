#include <nan.h>
NAN_METHOD(Resize);
NAN_METHOD(Open);

static void
v8tows(unsigned short *r, unsigned short *c, v8::Handle<v8::Value> size) {
	v8::Handle<v8::String> columns = Nan::New<v8::String>("columns").ToLocalChecked(),
		rows = Nan::New<v8::String>("rows").ToLocalChecked();
	*r = 24;
	*c = 80;
	if(size->IsObject()) {
		v8::Handle<v8::Object> obj = size->ToObject();
		if(obj->Has(columns))
			*c = obj->Get(columns)->Uint32Value();
		if(obj->Has(rows))
			*r = obj->Get(rows)->Uint32Value();
	}
}

static void
wstov8(v8::Handle<v8::Object> obj, unsigned short r, unsigned short c) {
	obj->Set(Nan::New<v8::String>("rows").ToLocalChecked(), Nan::New<v8::Integer>(r));
	obj->Set(Nan::New<v8::String>("columns").ToLocalChecked(), Nan::New<v8::Integer>(c));
}


void Init(v8::Handle<v8::Object> exports) {
	Nan::SetMethod(exports, "open", Open);
	Nan::SetMethod(exports, "resize", Resize);
}

NODE_MODULE(pty, Init)
