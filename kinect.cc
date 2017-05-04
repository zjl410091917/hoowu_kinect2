#include "kinect.h"
#include <iostream>

#include <NiTE.h>

#define GL_WIN_SIZE_X	1920
#define GL_WIN_SIZE_Y	1080

Nan::Persistent<v8::Function> Kinect::constructor;

Kinect::Kinect()
{}

Kinect::~Kinect()
{}

void Kinect::Init(v8::Local<v8::Object> exports)
{
  Nan::HandleScope scope;

  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Kinect").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "open", Open);
  Nan::SetPrototypeMethod(tpl, "close", Close);
  Nan::SetPrototypeMethod(tpl, "checkUserFrame", CheckUserFrame);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("Kinect").ToLocalChecked(), tpl->GetFunction());
}

void Kinect::New(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
  if (info.IsConstructCall())
  {
    Kinect *obj = new Kinect();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }
}

void Kinect::Open(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
  Kinect *obj = ObjectWrap::Unwrap<Kinect>(info.Holder());
  openni::Status rc = openni::OpenNI::initialize();

  if(rc != openni::STATUS_OK){
    
    info.GetReturnValue().Set(false);
    return;
  }
  
  rc = obj->m_device.open(openni::ANY_DEVICE);

  nite::NiTE::initialize();
  nite::Status niteRc = obj->m_userTracker.create(&(obj->m_device));
  if (niteRc != nite::STATUS_OK)
  {
    info.GetReturnValue().Set(false);
    return;
  }
  info.GetReturnValue().Set(true);  
}

void Kinect::CheckUserFrame(const Nan::FunctionCallbackInfo<v8::Value> &info){
    Kinect *obj = ObjectWrap::Unwrap<Kinect>(info.Holder());
    openni::VideoFrameRef depthFrame;

    nite::Status niteRc = obj->m_userTracker.readFrame(&(obj->m_userTrackerFrame));
    if(niteRc != nite::STATUS_OK){
      info.GetReturnValue().Set(false);
      return;
    }
    depthFrame = obj->m_userTrackerFrame.getDepthFrame();
    const nite::Array<nite::UserData>& users = obj->m_userTrackerFrame.getUsers();

    obj->m_nXRes = depthFrame.getVideoMode().getResolutionX();
	  obj->m_nYRes = depthFrame.getVideoMode().getResolutionY();

    v8::Local<v8::Object> v8Users = Nan::New<v8::Object>();

    // Nan::Set(v8Users, Nan::New<v8::String>("X").ToLocalChecked(), Nan::New<v8::Number>(obj->m_nXRes));
    // Nan::Set(v8Users, Nan::New<v8::String>("Y").ToLocalChecked(), Nan::New<v8::Number>(obj->m_nYRes));

    for(int i = 0; i < users.getSize(); ++i){
      const nite::UserData& user = users[i];
      if(user.isNew()){
        std::cout<<"New User:"<<user.getId()<<std::endl;
        obj->m_userTracker.startSkeletonTracking(user.getId());
        obj->m_userTracker.startPoseDetection(user.getId(), nite::POSE_CROSSED_HANDS);
      }else if(!user.isLost() && user.isVisible()){
        v8::Local<v8::Object> v8User = Nan::New<v8::Object>();
        
        Nan::Set(v8User, Nan::New<v8::String>("userId").ToLocalChecked(), Nan::New<v8::Number>(user.getId()));
        float x, y;

	      obj->m_userTracker.convertJointCoordinatesToDepth(user.getCenterOfMass().x, user.getCenterOfMass().y, user.getCenterOfMass().z, &x, &y);
      
        x /= (float)(obj->m_nXRes);
	      y /= (float)(obj->m_nYRes);
        
        Nan::Set(v8User, Nan::New<v8::String>("massX").ToLocalChecked(), Nan::New<v8::Number>(x > 0 ? x : 0));
        Nan::Set(v8User, Nan::New<v8::String>("massY").ToLocalChecked(), Nan::New<v8::Number>(y > 0 ? y : 0));
        
        bool isTracked = user.getSkeleton().getState() == nite::SKELETON_TRACKED;
        Nan::Set(v8User, Nan::New<v8::String>("isTracked").ToLocalChecked(), Nan::New<v8::Boolean>(isTracked));
        if(isTracked){          

          const nite::SkeletonJoint& head = user.getSkeleton().getJoint(nite::JOINT_HEAD);
          obj->m_userTracker.convertJointCoordinatesToDepth(head.getPosition().x, head.getPosition().y, head.getPosition().z, &x, &y);
          
          x /= (float)(obj->m_nXRes);
	        y /= (float)(obj->m_nYRes);
          Nan::Set(v8User, Nan::New<v8::String>("headX").ToLocalChecked(), Nan::New<v8::Number>(x > 0 ? x : 0));
          Nan::Set(v8User, Nan::New<v8::String>("headY").ToLocalChecked(), Nan::New<v8::Number>(y > 0 ? y : 0));              
        }

        Nan::Set(v8Users, Nan::New<v8::Number>(user.getId()), v8User);
      }
    }

    info.GetReturnValue().Set(v8Users);
}

void Kinect::Close(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
  std::cout<<"Close"<<std::endl;
  // nite::NiTE::shutdown();
  v8::Local<v8::Object> obj = Nan::New<v8::Object>();
  obj->Set(Nan::New("test").ToLocalChecked(), Nan::New(10));

  info.GetReturnValue().Set(obj);
}
