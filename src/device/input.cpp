#include "input.h"

#pragma comment(lib, "dinput8.lib")

#include "main.h"
#include "device/setting.h"
#if defined _DEBUGGER
#include "debugger/gui/window/screen_window.h"
#else  // defined _DEBUGGER
#include "window.h"
#endif // defined _DEBUGGER

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

namespace {

enum {
  JOY_UP = 0,
  JOY_DOWN,
  JOY_LEFT,
  JOY_RIGHT,
  JOY_BTN1,
  JOY_BTN2,
  JOY_BTN3,
  JOY_BTN4,
  JOY_BTN5,
  JOY_BTN6,
  JOY_BTN7,
  JOY_BTN8,
  JOY_BTN9,
  JOY_BTN10,
  JOY_BTN11,
  JOY_BTN12,
  JOY_BTN13,
  JOY_BTN14,
  JOY_BTN15,
  JOY_BTN16,
  JOY_EMPTY = 255,
};

#if defined _CONFIG

const struct KbdInfo {
  s32 code;
  s8  label[16];
} kKbdInfo[] = {
  DIK_TAB, "Tab",
  DIK_SPACE, "Space",
  DIK_RETURN, "Enter",

  DIK_NUMPAD0, "Num 0",
  DIK_NUMPAD1, "Num 1",
  DIK_NUMPAD2, "Num 2",
  DIK_NUMPAD3, "Num 3",
  DIK_NUMPAD4, "Num 4",
  DIK_NUMPAD5, "Num 5",
  DIK_NUMPAD6, "Num 6",
  DIK_NUMPAD7, "Num 7",
  DIK_NUMPAD8, "Num 8",
  DIK_NUMPAD9, "Num 9",
  DIK_NUMPADPERIOD, "Num .",
  DIK_NUMPADENTER, "Num Ent",
  DIK_NUMPADPLUS, "Num +",
  DIK_NUMPADMINUS, "Num -",
  DIK_NUMPADSLASH, "Num /",
  DIK_NUMPADSTAR, "Num *",
  
  DIK_INSERT, "Insert",
  DIK_HOME, "Home",
  DIK_PGUP, "PgUp",
  DIK_DELETE, "Delete",
  DIK_END, "End",
  DIK_PGDN, "PgDn",

  DIK_UP, "↑",
  DIK_DOWN, "↓",
  DIK_LEFT, "←",
  DIK_RIGHT, "→",

  DIK_1, "1",
  DIK_2, "2",
  DIK_3, "3",
  DIK_4, "4",
  DIK_5, "5",
  DIK_6, "6",
  DIK_7, "7",
  DIK_8, "8",
  DIK_9, "9",
  DIK_0, "0",
  DIK_MINUS, "-",
  DIK_PREVTRACK, "^",
  DIK_YEN, "\\",
  DIK_BACKSPACE, "BS",

  DIK_Q, "Q",
  DIK_W, "W",
  DIK_E, "E",
  DIK_R, "R",
  DIK_T, "T",
  DIK_Y, "Y",
  DIK_U, "U",
  DIK_I, "I",
  DIK_O, "O",
  DIK_P, "P",
  DIK_AT, "@",
  DIK_LBRACKET, "[",
  
  DIK_A, "A",
  DIK_S, "S",
  DIK_D, "D",
  DIK_F, "F",
  DIK_G, "G",
  DIK_H, "H",
  DIK_J, "J",
  DIK_K, "K",
  DIK_L, "L",
  DIK_SEMICOLON, ";",
  DIK_COLON, ":",
  DIK_RBRACKET, "]",
  
  DIK_Z, "Z",
  DIK_X, "X",
  DIK_C, "C",
  DIK_V, "V",
  DIK_B, "B",
  DIK_N, "N",
  DIK_M, "M",
  DIK_COMMA, ",",
  DIK_PERIOD, ".",
  DIK_SLASH, "/",
  DIK_BACKSLASH, "＼",
  255, "(None)",
  -1, ""
};

const struct JoyInfo {
  s32 code;
  s8  label[16];
} kJoyInfo[] = {
  JOY_UP, "↑",
  JOY_DOWN, "↓",
  JOY_LEFT, "←",
  JOY_RIGHT, "→",
  JOY_BTN1, "Btn1",
  JOY_BTN2, "Btn2",
  JOY_BTN3, "Btn3",
  JOY_BTN4, "Btn4",
  JOY_BTN5, "Btn5",
  JOY_BTN6, "Btn6",
  JOY_BTN7, "Btn7",
  JOY_BTN8, "Btn8",
  JOY_BTN9, "Btn9",
  JOY_BTN10, "Btn10",
  JOY_BTN11, "Btn11",
  JOY_BTN12, "Btn12",
  JOY_BTN13, "Btn13",
  JOY_BTN14, "Btn14",
  JOY_BTN15, "Btn15",
  JOY_BTN16, "Btn16",
  JOY_EMPTY, "(None)",
  -1, ""
};

#endif // defined _CONFIG

} // namespace

//extern SettingFileMgr* setting_file_mgr_;

InputMgr::InputMgr() :
    di_(NULL),
    di_dev_count_(0) {
  for (int i = 0; i < kMaxDeviceCount; i++) {
    di_dev_[i] = NULL;
  }
  target_device_[0] = -1;
  target_device_[1] = -1;
}

InputMgr::~InputMgr() {
  for (int i = 0; i < di_dev_count_; i++) {
    if (di_dev_ == NULL) continue;
    (*di_dev_[i])->Unacquire();
    (*di_dev_[i])->Release();
    delete di_dev_[i];
  }
  if (di_) di_->Release();
}

void InputMgr::init(HINSTANCE p_inst) {
  HRESULT hr;
  
  active_ = true;

  hr = DirectInput8Create(p_inst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&di_, NULL); 
  if FAILED(hr) {
  }

  di_dev_count_ = 0;

  // KeyBoard
  LPDIRECTINPUTDEVICE8* device = new LPDIRECTINPUTDEVICE8;
  hr = di_->CreateDevice(GUID_SysKeyboard, device, NULL);
  if FAILED(hr) {
  }
  hr = (*device)->SetDataFormat(&c_dfDIKeyboard);
  if FAILED(hr) {
  }
#ifdef _CONFIG
  hr = (*device)->SetCooperativeLevel(NULL,
                                      DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
#else
  hr = (*device)->SetCooperativeLevel(g_scrn->hwnd(),
                                      DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
#endif
  if FAILED(hr) {
  }
  hr = (*device)->Acquire();
  if FAILED(hr) {
  }
  addDevice(device);
  
  // JoyStick
  di_->EnumDevices(DI8DEVCLASS_GAMECTRL, enum_joysticks_callback, this, DIEDFL_ATTACHEDONLY);

  // setting.datで指定されているGUIDを持つデバイスを探し、
  // InputMgr内で管理するデバイスインデックスを取得する
  target_device_[0] = -1;
  target_device_[1] = -1;
  if (g_app.setting_file_mgr()->get_data()) {
    int dev1p = g_app.setting_file_mgr()->get_data()->select_device(0) - 1;
    int dev2p = g_app.setting_file_mgr()->get_data()->select_device(1) - 1;
    for (int i = 0; i < di_dev_count_; i++) {
      if (dev1p >= 0 &&
          memcmp(getDeviceGuid(i),
                 &g_app.setting_file_mgr()->get_data()->device(dev1p)->guid,
                 sizeof(GUID)) == 0) {
        target_device_[0] = i;
      }
      if (dev2p >= 0 &&
          memcmp(getDeviceGuid(i),
                 &g_app.setting_file_mgr()->get_data()->device(dev2p)->guid,
                 sizeof(GUID)) == 0) {
        target_device_[1] = i;
      }
    }
  }
}

void InputMgr::addDevice(LPDIRECTINPUTDEVICE8* p_device) {
  if (di_dev_count_ < kMaxDeviceCount) {
    di_dev_[di_dev_count_++] = p_device;
  }
}

char* InputMgr::getDeviceName(int p_idx) {
  static DIDEVICEINSTANCE di_dev_inst;
  ZeroMemory(&di_dev_inst, sizeof(DIDEVICEINSTANCE));
  di_dev_inst.dwSize = sizeof(DIDEVICEINSTANCE);
  HRESULT hr = (*di_dev_[p_idx])->GetDeviceInfo(&di_dev_inst);
  if (FAILED(hr)) {
    return "";
  }
  return di_dev_inst.tszInstanceName;
}

GUID* InputMgr::getDeviceGuid(int p_idx) {
  static DIDEVICEINSTANCE di_dev_inst;
  ZeroMemory(&di_dev_inst, sizeof(DIDEVICEINSTANCE));
  di_dev_inst.dwSize = sizeof(DIDEVICEINSTANCE);
  HRESULT hr = (*di_dev_[p_idx])->GetDeviceInfo(&di_dev_inst);
  if (FAILED(hr)) {
    return NULL;
  }
  return &di_dev_inst.guidInstance;
}

void InputMgr::activate() {
#if _CONFIG
  active_ = true;
#else
  if (g_app.setting_file_mgr() &&
      g_app.setting_file_mgr()->get_data()->input_in_background() == false) {
    active_ = g_scrn->active();
  } else {
    active_ = true;
  }
#endif
}

void InputMgr::update() {
  if (!active_) return;

  HRESULT hr;
  for (int i = 0; i < di_dev_count_; i++) {
    if (i == 0) {
      // キーボードによる入力情報の取得
      if (di_dev_[i]) {
        memset(key_, 0, 256);
        hr = (*di_dev_[i])->GetDeviceState(256, (void*)key_);
        if FAILED(hr) {
        }
      }
    } else {
      // パッドによる入力情報の取得
      if (di_dev_[i]) {
        hr = (*di_dev_[i])->Poll();
        if (FAILED(hr)) {
          hr = (*di_dev_[i])->Acquire();
        }
        hr = (*di_dev_[i])->GetDeviceState(sizeof(DIJOYSTATE), &joy_[i - 1]);
        if FAILED(hr) {
        }
      }
    }
  }
}

u32 InputMgr::getTargetState(u32 p_side) {
  u32 result = 0;
  if (target_device_[p_side] >= 0) {
    const u32 input_code[16] = {
      0x00001000, 0x00004000, 0x00008000, 0x00002000,
      0x00000020, 0x00000040, 0x00000010, 0x00000080,
      0x00000004, 0x00000008, 0x00000001, 0x00000002,
      0x00000200, 0x00000400, 0x00000800, 0x00000100
    };

    int setting_devidx =
      g_app.setting_file_mgr()->get_data()->select_device(p_side) - 1;
    if (setting_devidx >= 0 &&
        setting_devidx < g_app.setting_file_mgr()->get_data()->device_count()) {
      for (int i = 0; i < 16; i++) {
        int keycode =
          g_app.setting_file_mgr()->get_data()->device(setting_devidx)->key[p_side][i];
        int analog_sense =
          g_app.setting_file_mgr()->get_data()->device_analog_sense(p_side);

        if (getState(target_device_[p_side], keycode, analog_sense)) {
          result |= input_code[i];
        }
      }
    }
  }
  return result;
}

bool InputMgr::getState(int p_devidx, int p_keycode, int p_analog_sense) {
  if (p_devidx == 0) {
    return !!(key_[p_keycode] & 0x80);
  } else {
    switch (p_keycode) {
    case JOY_RIGHT: return (joy_[p_devidx - 1].lX >  100 - p_analog_sense);
    case JOY_LEFT:  return (joy_[p_devidx - 1].lX < -100 + p_analog_sense);
    case JOY_DOWN:  return (joy_[p_devidx - 1].lY >  100 - p_analog_sense);
    case JOY_UP:    return (joy_[p_devidx - 1].lY < -100 + p_analog_sense);
    case JOY_BTN1:  return !!joy_[p_devidx - 1].rgbButtons[0];
    case JOY_BTN2:  return !!joy_[p_devidx - 1].rgbButtons[1];
    case JOY_BTN3:  return !!joy_[p_devidx - 1].rgbButtons[2];
    case JOY_BTN4:  return !!joy_[p_devidx - 1].rgbButtons[3];
    case JOY_BTN5:  return !!joy_[p_devidx - 1].rgbButtons[4];
    case JOY_BTN6:  return !!joy_[p_devidx - 1].rgbButtons[5];
    case JOY_BTN7:  return !!joy_[p_devidx - 1].rgbButtons[6];
    case JOY_BTN8:  return !!joy_[p_devidx - 1].rgbButtons[7];
    case JOY_BTN9:  return !!joy_[p_devidx - 1].rgbButtons[8];
    case JOY_BTN10: return !!joy_[p_devidx - 1].rgbButtons[9];
    case JOY_BTN11: return !!joy_[p_devidx - 1].rgbButtons[10];
    case JOY_BTN12: return !!joy_[p_devidx - 1].rgbButtons[11];
    case JOY_BTN13: return !!joy_[p_devidx - 1].rgbButtons[12];
    case JOY_BTN14: return !!joy_[p_devidx - 1].rgbButtons[13];
    case JOY_BTN15: return !!joy_[p_devidx - 1].rgbButtons[14];
    case JOY_BTN16: return !!joy_[p_devidx - 1].rgbButtons[15];
    }
  }
  // 未知のキーコードorデバイス
  return false;
}

#if defined _CONFIG

int InputMgr::getKeyCode(int p_devidx, int p_analog_sense) {
  if (p_devidx == 0) {
    // 押されているキーのコード（最初に検出されたもの）を返す
    for (int i = 0; kKbdInfo[i].code != -1; i++) {
      if (key_[kKbdInfo[i].code] & 0x80) {
        return kKbdInfo[i].code;
      }
    }
  } else {
    // 押されているボタンのコード（最初に検出されたもの）を返す
    if (joy_[p_devidx - 1].lX >  100 - p_analog_sense) return JOY_RIGHT;
    if (joy_[p_devidx - 1].lX < -100 + p_analog_sense) return JOY_LEFT;
    if (joy_[p_devidx - 1].lY >  100 - p_analog_sense) return JOY_DOWN;
    if (joy_[p_devidx - 1].lY < -100 + p_analog_sense) return JOY_UP;
    if (joy_[p_devidx - 1].rgbButtons[0])  return JOY_BTN1;
    if (joy_[p_devidx - 1].rgbButtons[1])  return JOY_BTN2;
    if (joy_[p_devidx - 1].rgbButtons[2])  return JOY_BTN3;
    if (joy_[p_devidx - 1].rgbButtons[3])  return JOY_BTN4;
    if (joy_[p_devidx - 1].rgbButtons[4])  return JOY_BTN5;
    if (joy_[p_devidx - 1].rgbButtons[5])  return JOY_BTN6;
    if (joy_[p_devidx - 1].rgbButtons[6])  return JOY_BTN7;
    if (joy_[p_devidx - 1].rgbButtons[7])  return JOY_BTN8;
    if (joy_[p_devidx - 1].rgbButtons[8])  return JOY_BTN9;
    if (joy_[p_devidx - 1].rgbButtons[9])  return JOY_BTN10;
    if (joy_[p_devidx - 1].rgbButtons[10]) return JOY_BTN11;
    if (joy_[p_devidx - 1].rgbButtons[11]) return JOY_BTN12;
    if (joy_[p_devidx - 1].rgbButtons[12]) return JOY_BTN13;
    if (joy_[p_devidx - 1].rgbButtons[13]) return JOY_BTN14;
    if (joy_[p_devidx - 1].rgbButtons[14]) return JOY_BTN15;
    if (joy_[p_devidx - 1].rgbButtons[15]) return JOY_BTN16;
  }
  return -1;
}

const s8* InputMgr::getKeyName(int p_devidx, int p_analog_sense) {
  if (p_devidx == 0) {
    // 押されているキーの名前（最初に検出されたもの）を返す
    for (int i = 0; kKbdInfo[i].code != -1; i++) {
      if (key_[kKbdInfo[i].code] & 0x80) {
        return kKbdInfo[i].label;
      }
    }
  } else {
    // 押されているボタンのコード（最初に検出されたもの）を返す
    if (joy_[p_devidx - 1].lX >  100 - p_analog_sense) return kJoyInfo[JOY_RIGHT].label;
    if (joy_[p_devidx - 1].lX < -100 + p_analog_sense) return kJoyInfo[JOY_LEFT].label;
    if (joy_[p_devidx - 1].lY >  100 - p_analog_sense) return kJoyInfo[JOY_DOWN].label;
    if (joy_[p_devidx - 1].lY < -100 + p_analog_sense) return kJoyInfo[JOY_UP].label;
    if (joy_[p_devidx - 1].rgbButtons[0])  return kJoyInfo[JOY_BTN1].label;
    if (joy_[p_devidx - 1].rgbButtons[1])  return kJoyInfo[JOY_BTN2].label;
    if (joy_[p_devidx - 1].rgbButtons[2])  return kJoyInfo[JOY_BTN3].label;
    if (joy_[p_devidx - 1].rgbButtons[3])  return kJoyInfo[JOY_BTN4].label;
    if (joy_[p_devidx - 1].rgbButtons[4])  return kJoyInfo[JOY_BTN5].label;
    if (joy_[p_devidx - 1].rgbButtons[5])  return kJoyInfo[JOY_BTN6].label;
    if (joy_[p_devidx - 1].rgbButtons[6])  return kJoyInfo[JOY_BTN7].label;
    if (joy_[p_devidx - 1].rgbButtons[7])  return kJoyInfo[JOY_BTN8].label;
    if (joy_[p_devidx - 1].rgbButtons[8])  return kJoyInfo[JOY_BTN9].label;
    if (joy_[p_devidx - 1].rgbButtons[9])  return kJoyInfo[JOY_BTN10].label;
    if (joy_[p_devidx - 1].rgbButtons[10]) return kJoyInfo[JOY_BTN11].label;
    if (joy_[p_devidx - 1].rgbButtons[11]) return kJoyInfo[JOY_BTN12].label;
    if (joy_[p_devidx - 1].rgbButtons[12]) return kJoyInfo[JOY_BTN13].label;
    if (joy_[p_devidx - 1].rgbButtons[13]) return kJoyInfo[JOY_BTN14].label;
    if (joy_[p_devidx - 1].rgbButtons[14]) return kJoyInfo[JOY_BTN15].label;
    if (joy_[p_devidx - 1].rgbButtons[15]) return kJoyInfo[JOY_BTN16].label;
  }
  return NULL;
}

const s8* InputMgr::key2str(int p_code) {
  for (int i = 0; kKbdInfo[i].code != -1; i++) {
    if (kKbdInfo[i].code == p_code) {
      return kKbdInfo[i].label;
    }
  }
  return NULL;
}

int InputMgr::str2key(s8* p_str) {
  for (int i = 0; kKbdInfo[i].code != -1; i++) {
    if (strcmp(kKbdInfo[i].label, p_str) == 0) {
      return kKbdInfo[i].code;
    }
  }
  return -1;
}

const s8* InputMgr::joy2str(int p_code) {
  for (int i = 0; kJoyInfo[i].code != -1; i++) {
    if (kJoyInfo[i].code == p_code) {
      return kJoyInfo[i].label;
    }
  }
  return NULL;
}

int InputMgr::getEmptyCode(int p_devidx) {
  return p_devidx == 0 ? 255 : JOY_EMPTY;
}

#endif // defined _CONFIG

int CALLBACK InputMgr::enum_joysticks_callback(
    const DIDEVICEINSTANCE* p_di_dev_inst,
    void* p_context) {
  InputMgr* input_mgr = (InputMgr*)p_context;

  // たくさんあったら途中でやめる
  if (input_mgr->device_count() >= InputMgr::kMaxDeviceCount) {
    return DIENUM_STOP;
  }

  HRESULT hr;
  LPDIRECTINPUTDEVICE8* didev = new LPDIRECTINPUTDEVICE8;
  if (didev == NULL) {
    return DIENUM_STOP;
  }

  hr = input_mgr->di()->CreateDevice(p_di_dev_inst->guidInstance, didev, NULL);
  if (FAILED(hr)) {
    (*didev)->Release();
    delete didev;
    return DIENUM_CONTINUE;
  }
  
  hr = (*didev)->SetDataFormat(&c_dfDIJoystick);
  if (FAILED(hr)) {
    (*didev)->Release();
    delete didev;
    return DIENUM_CONTINUE;
  }
#ifdef _CONFIG
  hr = (*didev)->SetCooperativeLevel(NULL,
                                     DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
#else
  hr = (*didev)->SetCooperativeLevel(g_scrn->hwnd(),
                                     DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
#endif

  if (FAILED(hr))  {
    (*didev)->Release();
    delete didev;
    return DIENUM_CONTINUE;
  }
  
  hr = (*didev)->EnumObjects(enum_axis_callback, didev, DIDFT_AXIS);
  if (FAILED(hr)) {
    (*didev)->Release();
    delete didev;
    return DIENUM_CONTINUE;
  }

  hr = (*didev)->Acquire();
  if (FAILED(hr)) {
    (*didev)->Release();
    delete didev;
    return DIENUM_CONTINUE;
  }
  
  input_mgr->addDevice(didev);

  return DIENUM_CONTINUE;
}

int CALLBACK InputMgr::enum_axis_callback(
    LPCDIDEVICEOBJECTINSTANCE p_di_dev_obj_inst,
    void* p_vref) {
  // 軸の値の範囲を設定（-1000～1000の範囲で指定可能）
  DIPROPRANGE di_prop_range;
  ZeroMemory(&di_prop_range, sizeof(DIPROPRANGE));
  di_prop_range.diph.dwSize = sizeof(DIPROPRANGE); 
  di_prop_range.diph.dwHeaderSize = sizeof(di_prop_range.diph); 
  di_prop_range.diph.dwObj  = p_di_dev_obj_inst->dwType;
  di_prop_range.diph.dwHow  = DIPH_BYID;
  di_prop_range.lMin        = -100;
  di_prop_range.lMax        =  100;
  HRESULT hr = (*((LPDIRECTINPUTDEVICE8*)p_vref))->SetProperty(DIPROP_RANGE, &di_prop_range.diph);
  if (FAILED(hr)) {
    return DIENUM_STOP;
  }
  return DIENUM_CONTINUE;
}
