/*
 * pty.cpp
 * Copyright (C) 2014 tox <tox@rootkit>
 *
 * Distributed under terms of the MIT license.
 */

#include <node.h>
#include <nan.h>
#include <alloca.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

using namespace v8;

#if defined(__unix__)
#	include "unix.cpp"
#elif defined(_WIN32)
#	include "win.cpp"
#endif

void Init(Handle<Object> exports) {
	Handle<FunctionTemplate> ptx = NanNew<FunctionTemplate>(Open);;
	exports->Set(NanNew<String>("open"), ptx->GetFunction());
	ptx->InstanceTemplate()->SetInternalFieldCount(1);


	ptx->PrototypeTemplate()->Set(NanNew<String>("resize"),
			NanNew<FunctionTemplate>(Resize)->GetFunction());
}

NODE_MODULE(pty, Init)
