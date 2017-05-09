#include <nan.h>
#include <uv.h>
#include <unistd.h>
#include <iostream>
#include <NiTE.h>
// #include "kinect.h"

// void InitAll(v8::Local<v8::Object> exports) {
//   Kinect::Init(exports);
// }

using namespace v8;

#define USER_COUNT 10

openni::Device m_device;
nite::UserTracker m_userTracker;
nite::UserTrackerFrameRef m_userTrackerFrame;

Nan::Callback *m_pOpenTrackerCallback;

uv_mutex_t m_mTrackerMutex;

uv_async_t m_aTrackerAsync;
uv_thread_t m_tTrackerThread;
bool m_bTrackerThreadRunning = false;

typedef struct
{
  bool isTracked;
  int userId;
  float massX;
  float massY;
  float headX;
  float headY;
} TrackUser;

TrackUser m_trackUsers[USER_COUNT];

NAN_METHOD(OpenFunction)
{
  openni::Status rc = openni::OpenNI::initialize();

  if (rc != openni::STATUS_OK)
  {
    info.GetReturnValue().Set(false);
    return;
  }

  rc = m_device.open(openni::ANY_DEVICE);
  if (rc != openni::STATUS_OK)
  {
    info.GetReturnValue().Set(false);
    return;
  }

  nite::NiTE::initialize();
  nite::Status niteRc = m_userTracker.create(&m_device);
  if (niteRc != nite::STATUS_OK)
  {
    info.GetReturnValue().Set(false);
    return;
  }

  info.GetReturnValue().Set(true);
}
NAN_METHOD(CloseFunction)
{
  m_userTracker.destroy();
  nite::NiTE::shutdown();
  openni::OpenNI::shutdown();
  info.GetReturnValue().Set(true);
}

NAUV_WORK_CB(TrackerProgress_)
{
  Nan::HandleScope scope;
  uv_mutex_lock(&m_mTrackerMutex);
  if (m_pOpenTrackerCallback != NULL)
  {
    bool flag = false;
    v8::Local<v8::Object> v8Users = Nan::New<v8::Object>();
    for (int i = 0; i < USER_COUNT; ++i)
    {
      if (m_trackUsers[i].userId > 0)
      {
        flag = true;
        v8::Local<v8::Object> v8User = Nan::New<v8::Object>();
        Nan::Set(v8User, Nan::New<v8::String>("userId").ToLocalChecked(), Nan::New<v8::Number>(m_trackUsers[i].userId));
        Nan::Set(v8User, Nan::New<v8::String>("massX").ToLocalChecked(), Nan::New<v8::Number>(m_trackUsers[i].massX));
        Nan::Set(v8User, Nan::New<v8::String>("massY").ToLocalChecked(), Nan::New<v8::Number>(m_trackUsers[i].massY));
        Nan::Set(v8User, Nan::New<v8::String>("isTracked").ToLocalChecked(), Nan::New<v8::Boolean>(m_trackUsers[i].isTracked));
        if (m_trackUsers[i].isTracked)
        {
          Nan::Set(v8User, Nan::New<v8::String>("headX").ToLocalChecked(), Nan::New<v8::Number>(m_trackUsers[i].headX));
          Nan::Set(v8User, Nan::New<v8::String>("headY").ToLocalChecked(), Nan::New<v8::Number>(m_trackUsers[i].headY));
        }
        Nan::Set(v8Users, Nan::New<v8::Number>(m_trackUsers[i].userId), v8User);
      }
    }

    v8::Local<v8::Value> argv[] = {v8Users};
    if (flag)
    {
      m_pOpenTrackerCallback->Call(1, argv);
    }
  }
  uv_mutex_unlock(&m_mTrackerMutex);
}

void TrackerThreadLoop(void *arg)
{
  int m_nXRes;
  int m_nYRes;
  float x, y;

  while (1)
  {
    usleep(50000);
    openni::VideoFrameRef depthFrame;
    // std::cout << "TrackerThreadLoop1" << std::endl;
    nite::Status niteRc = m_userTracker.readFrame(&m_userTrackerFrame);
    // std::cout << "TrackerThreadLoop2" << std::endl;
    if (niteRc != nite::STATUS_OK)
    {
      std::cout << "readFrame failed" << std::endl;
      continue;
    }
    depthFrame = m_userTrackerFrame.getDepthFrame();
    // std::cout << "TrackerThreadLoop3" << std::endl;
    const nite::Array<nite::UserData> &users = m_userTrackerFrame.getUsers();
    // std::cout << "TrackerThreadLoop4" << std::endl;
    m_nXRes = depthFrame.getVideoMode().getResolutionX();
    m_nYRes = depthFrame.getVideoMode().getResolutionY();

    TrackUser temp[USER_COUNT];
    memset(temp, 0, sizeof(TrackUser) * USER_COUNT);

    int index = 0;
    for (int i = 0; i < users.getSize(); ++i)
    {
      const nite::UserData &user = users[i];
      if (user.isNew())
      {
        std::cout << "New User:" << user.getId() << std::endl;
        m_userTracker.startSkeletonTracking(user.getId());
        m_userTracker.startPoseDetection(user.getId(), nite::POSE_CROSSED_HANDS);
      }
      else if (!user.isLost() && user.isVisible())
      {
        temp[index].userId = user.getId();

        m_userTracker.convertJointCoordinatesToDepth(user.getCenterOfMass().x, user.getCenterOfMass().y, user.getCenterOfMass().z, &x, &y);

        x /= (float)(m_nXRes);
        y /= (float)(m_nYRes);

        temp[index].massX = x > 0 ? x : 0;
        temp[index].massY = y > 0 ? y : 0;

        bool isTracked = user.getSkeleton().getState() == nite::SKELETON_TRACKED;
        temp[index].isTracked = isTracked;

        if (isTracked)
        {
          const nite::SkeletonJoint &head = user.getSkeleton().getJoint(nite::JOINT_HEAD);
          m_userTracker.convertJointCoordinatesToDepth(head.getPosition().x, head.getPosition().y, head.getPosition().z, &x, &y);

          x /= (float)(m_nXRes);
          y /= (float)(m_nYRes);
          temp[index].headX = x > 0 ? x : 0;
          temp[index].headY = y > 0 ? y : 0;
        }

        index++;
      }
    }

    bool flag = true;
    uv_mutex_lock(&m_mTrackerMutex);
    flag = m_bTrackerThreadRunning;
    memcpy(m_trackUsers, temp, sizeof(TrackUser) * USER_COUNT);
    uv_mutex_unlock(&m_mTrackerMutex);
    uv_async_send(&m_aTrackerAsync);
    if (!flag)
    {
      break;
    }
  }
}

NAN_METHOD(OpenTrackerFunction)
{
  uv_mutex_lock(&m_mTrackerMutex);
  if (m_pOpenTrackerCallback)
  {
    delete m_pOpenTrackerCallback;
    m_pOpenTrackerCallback = NULL;
  }
  m_pOpenTrackerCallback = new Nan::Callback(info[0].As<Function>());

  if (!m_bTrackerThreadRunning)
  {
    m_bTrackerThreadRunning = true;
    uv_async_init(uv_default_loop(), &m_aTrackerAsync, TrackerProgress_);
    uv_thread_create(&m_tTrackerThread, TrackerThreadLoop, NULL);
  }
  uv_mutex_unlock(&m_mTrackerMutex);

  info.GetReturnValue().Set(true);
}

NAN_METHOD(CloseTrackerFunction)
{
  uv_mutex_lock(&m_mTrackerMutex);
  if (m_pOpenTrackerCallback)
  {
    delete m_pOpenTrackerCallback;
    m_pOpenTrackerCallback = NULL;
  }

  m_bTrackerThreadRunning = false;
  uv_mutex_unlock(&m_mTrackerMutex);

  info.GetReturnValue().Set(true);
}

NAN_MODULE_INIT(Init)
{
  Nan::Set(target, Nan::New<String>("open").ToLocalChecked(),
           Nan::New<FunctionTemplate>(OpenFunction)->GetFunction());
  Nan::Set(target, Nan::New<String>("close").ToLocalChecked(),
           Nan::New<FunctionTemplate>(CloseFunction)->GetFunction());
  Nan::Set(target, Nan::New<String>("openTracker").ToLocalChecked(),
           Nan::New<FunctionTemplate>(OpenTrackerFunction)->GetFunction());
  Nan::Set(target, Nan::New<String>("closeTracker").ToLocalChecked(),
           Nan::New<FunctionTemplate>(CloseTrackerFunction)->GetFunction());
}

NODE_MODULE(kinect2, Init)
