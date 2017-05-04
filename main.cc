#include <nan.h>
#include "kinect.h"

void InitAll(v8::Local<v8::Object> exports) {
  Kinect::Init(exports);
}

NODE_MODULE(addon, InitAll)
