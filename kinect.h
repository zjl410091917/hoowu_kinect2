#ifndef KINECT_H
#define KINECT_H

#include <nan.h>
#include <NiTE.h>

class Kinect : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

 private:
  explicit Kinect();
  ~Kinect();

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void GetValue(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void PlusOne(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Multiply(const Nan::FunctionCallbackInfo<v8::Value>& info);
  

  static void Open(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Close(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void CheckUserFrame(const Nan::FunctionCallbackInfo<v8::Value>& info);
  
  static Nan::Persistent<v8::Function> constructor;
    
  openni::Device  m_device;
  nite::UserTracker m_userTracker;
  nite::UserTrackerFrameRef m_userTrackerFrame;

  int m_nXRes;
	int m_nYRes;
};

#endif
