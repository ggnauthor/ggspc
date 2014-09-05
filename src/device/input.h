#ifndef _ggsext_input
#define _ggsext_input

#define DIRECTINPUT_VERSION  0x0800
#include <dinput.h>

#include "main.h"

class InputMgr {
public:
  enum { kMaxDeviceCount = 8 };
// constructor/destructor
  InputMgr();
  ~InputMgr();

// member function
  void init(HINSTANCE p_inst);
  void addDevice(LPDIRECTINPUTDEVICE8* p_device);

  char* getDeviceName(int p_idx);
  GUID* getDeviceGuid(int p_idx);
  
  void activate();
  void update();
  u32  getTargetState(u32 p_side);
  bool getState(int p_devidx, int p_keycode, int p_analog_sense = 50);

#if defined _CONFIG
  int       getKeyCode(int p_devidx, int p_analog_sense = 50);
  const s8* getKeyName(int p_devidx, int p_analog_sense = 50);
  
  static const s8* key2str(int p_code);
  static int       str2key(s8* p_str);
  static const s8* joy2str(int p_code);
  static int       getEmptyCode(int p_devidx);
#endif // defined _CONFIG

  LPDIRECTINPUT8 di() { return di_; }
  int device_count() { return di_dev_count_; }

private:
  static int CALLBACK enum_joysticks_callback(
    const DIDEVICEINSTANCE* pdidInstance, void* pContext);
  static int CALLBACK enum_axis_callback(
    LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);

  LPDIRECTINPUT8        di_;
  int                   di_dev_count_;
  LPDIRECTINPUTDEVICE8* di_dev_[kMaxDeviceCount];

  u8         key_[256];
  DIJOYSTATE joy_[kMaxDeviceCount - 1];

  int        target_device_[2];

  bool active_;
};

#endif // _ggsext_input
