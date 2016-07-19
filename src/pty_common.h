void Init(v8::Handle<v8::Object> exports) {
	Nan::SetMethod(exports, "open", Open);
	Nan::SetMethod(exports, "resize", Resize);
}

NODE_MODULE(pty, Init)
