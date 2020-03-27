#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#include "FREKVENS.h"

using namespace v8;
using namespace node;

void start(const FunctionCallbackInfo<Value>& args) {
  Isolate *pIsolate = args.GetIsolate();

  Persistent<Function> switchEventCallback;
  switchEventCallback.Reset(pIsolate, Local<Function>::Cast(args[0]));

  FREKVENS::start([&pIsolate, &switchEventCallback](const char *szEventName) -> void {
    Handle<Value> argv[1];
    argv[0] = String::NewFromUtf8(pIsolate, szEventName);

    Local<Function>::New(pIsolate, switchEventCallback)->Call(pIsolate->GetCurrentContext()->Global(), 1, argv);
  });

  args.GetReturnValue().Set(Undefined(pIsolate));
}

void stop(const FunctionCallbackInfo<Value>& args) {
  Isolate *pIsolate = args.GetIsolate();

  FREKVENS::stop();

  args.GetReturnValue().Set(Undefined(pIsolate));
}

void render(const FunctionCallbackInfo<Value>& args) {
  Isolate *pIsolate = args.GetIsolate();

  const char *pixels = Buffer::Data(args[0]->ToObject());

  FREKVENS::render(pixels);

  args.GetReturnValue().Set(Undefined(pIsolate));
}

void init(Local<Object> exports, Local<Object> module) {
  NODE_SET_METHOD(exports, "start", start);
  NODE_SET_METHOD(exports, "stop", stop);
  NODE_SET_METHOD(exports, "render", render);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, init)
