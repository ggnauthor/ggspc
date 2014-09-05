#ifndef _ggsext_setting
#define _ggsext_setting

#include <stdlib.h>
#include <guiddef.h>
#include <assert.h>

#include "main.h"

#include "device/input.h"

#pragma pack(push, 1)

typedef struct {
  GUID guid;
  u32  key[2][16];
} DeviceEntry;

class SettingFileData {
  friend class ggscfg_GeneralSheet;
  friend class ggscfg_NetworkSheet;
  friend class ggscfg_EditCommentDialog;

public:
  enum {
    kDenyMethod_IPAddress = 0,
    kDenyMethod_ClientCode = 1,
  };

  enum {
    kDisplayMode_Window = 0,
    kDisplayMode_FullScreenCoop = 1,
    kDisplayMode_FullScreenExcl = 2,
  };

  enum {
    kRankF = 0,
    kRankE,
    kRankD,
    kRankC,
    kRankB,
    kRankA,
    kRankS,
  };

  enum {
    kPlayerNameSize = 32,
    kTripKeySize = 32,
    kCommentSize = 32,
    kMaxLobbyAddressCount = 8,
  };

  virtual void init();
  virtual void upgrade(u16 p_fmt, void* p_data) = 0;

  s8* get_data() { return (s8*)&format_version_; }

// accessor/mutator
public:
  u16 format_version() { return format_version_; }
  u32 data_offset() { return (u32)&format_version_ - (u32)this; }

  // General Setting
  //   Graphics
  int   graph_fullscreen() { return graph_fullscreen_; }
  bool  graph_frame_skip() { return graph_frame_skip_; }
  bool  graph_wait_vsync() { return graph_wait_vsync_; }
  int   graph_color_depth() { return graph_color_depth_; }
  //   Sound
  bool  bgm_enable() { return bgm_enable_; }
  u32   bgm_volume() { return bgm_volume_; }
  bool  se_enable() { return se_enable_; }
  u32   se_volume() { return se_volume_; }
  bool  voice_enable() { return voice_enable_; }
  u32   voice_volume() { return voice_volume_; }
  bool  play_in_background() { return play_in_background_; }
  //   Misc
  bool  unlock_all_contents() { return unlock_all_contents_; }
  bool  display_fps() { return display_fps_; }
  bool  disable_movie() { return disable_movie_; }
  //   Device
  int          device_count() { return device_count_; }
  DeviceEntry* device(int p_idx) { return &device_[p_idx]; }
  int          select_device(int p_idx) { return select_device_[p_idx]; }
  // NetworkSetting
  //   Connection
  u32   port() { return port_; }
  int   delay_frame() { return delay_frame_; }
  int   rollback_frame() { return rollback_frame_; }
  //   Rule
  bool  enable_ex() { return enable_ex_; }
  bool  enable_sp() { return enable_sp_; }
  int   game_rounds() { return game_rounds_; }
  //   Watch
  bool  allow_watch() { return allow_watch_; }
  bool  allow_intrusion() { return allow_intrusion_; }
  bool  save_watch_replay() { return save_watch_replay_; }
  int   max_relay_node_count() { return max_relay_node_count_; }
  //   Lobby
  int   lobby_select_script() { return lobby_select_script_; }
  int   lobby_url_count() { return lobby_url_count_; }
  const s8* lobby_url(int p_idx) { return lobby_url_[p_idx]; }
  const s8* lobby_font_face() { return lobby_font_face_; }
  int   lobby_font_size() { return lobby_font_size_; }
  bool  enable_lobby_font_antialias() { return enable_lobby_font_antialias_; }
  //   Deny
  u32   deny_method() { return deny_method_; }
  bool  deny_slow_user() { return deny_slow_user_; }
  bool  deny_disconnecter() { return deny_disconnecter_; }
  //   Profile
  const s8* player_name() { return player_name_; }
  const s8* trip_key() { return trip_key_; }
  const s8* comment() { return comment_; }
  int   rank() { return rank_; }
  int   win_count() { return win_count_; }
  int   game_count() { return game_count_; }
  float win_rate() { return win_rate_; }
  float slow_rate() { return slow_rate_; }
  float disconnect_rate() { return disconnect_rate_; }
  
protected:
  u16 format_version_;

  // General Setting
  //   Graphics
  int   graph_fullscreen_;
  bool  graph_frame_skip_;
  bool  graph_wait_vsync_;
  int   graph_color_depth_;
  //   Sound
  bool  bgm_enable_;
  u32   bgm_volume_;
  bool  se_enable_;
  u32   se_volume_;
  bool  voice_enable_;
  u32   voice_volume_;
  bool  play_in_background_;
  //   Misc
  bool  unlock_all_contents_;
  bool  display_fps_;
  bool  disable_movie_;
  //   Device
  int         device_count_;
  DeviceEntry device_[InputMgr::kMaxDeviceCount];
  int         select_device_[2];

  // NetworkSetting
  //   Connection
  u32   port_;
  int   delay_frame_;
  int   rollback_frame_;
  //   Rule
  bool  enable_ex_;
  bool  enable_sp_;
  int   game_rounds_;
  //   Watch
  bool  allow_watch_;
  bool  allow_intrusion_;
  bool  save_watch_replay_;
  int   max_relay_node_count_;
  //   Lobby
  int   lobby_select_script_;
  int   lobby_url_count_;
  s8    lobby_url_[kMaxLobbyAddressCount][256];
  s8    lobby_font_face_[256];
  int   lobby_font_size_;
  bool  enable_lobby_font_antialias_;
  //   Deny
  u32   deny_method_;
  bool  deny_slow_user_;
  bool  deny_disconnecter_;
  //   Profile
  s8    player_name_[kPlayerNameSize];
  s8    trip_key_[kTripKeySize];
  s8    comment_[kCommentSize];
  int   rank_;
  int   win_count_;
  int   game_count_;
  float win_rate_;
  float slow_rate_;
  float disconnect_rate_;
};

// -----------------------------------------------------------------------------
// Version 0.9.0.0 (Initial Development Version)
class SettingFileData0900 : public SettingFileData {
  friend class ggscfg_GeneralSheet;
  friend class ggscfg_NetworkSheet;
  friend class ggscfg_EditCommentDialog;

public:
  enum { kThisVersion = 0x0900 };

  virtual void init() {
    SettingFileData::init();
    format_version_ = kThisVersion;
  }

  virtual void upgrade(u16 p_fmt, void* p_data) {
    // nop
  }
};

// -----------------------------------------------------------------------------
// Version 1.0.0.0 (Release Candidate Version)
#define kThisVersionClass     SettingFileData1000
#define kPreviousVersionClass SettingFileData0900
class kThisVersionClass : public kPreviousVersionClass {
  friend class ggscfg_GeneralSheet;
  friend class ggscfg_NetworkSheet;
  friend class ggscfg_EditCommentDialog;

public:
  enum { kThisVersion = 0x1000 };

  virtual void init() {
    kPreviousVersionClass::init();
    partial_init();
  }

  virtual void upgrade(u16 p_fmt, void* p_data) {
    // さらに前のバージョンが存在する場合は先に適用する
    if (p_fmt <= kPreviousVersionClass::kThisVersion) {
      kPreviousVersionClass::upgrade(p_fmt, p_data);
      partial_init();
    } else {
      memcpy((s8*)this + data_offset(), p_data,
             sizeof(kThisVersionClass) - data_offset());
    }
  }

// accessor/mutator
public:
  const s8* graph_display_device() { return graph_display_device_; }

  bool disable_screensaver() { return disable_screensaver_; }
  s32  device_analog_sense(int p_idx) { return device_analog_sense_[p_idx]; }

private:
  void partial_init() {
    format_version_ = kThisVersion;
    memset(graph_display_device_, 0x00, 32);
    disable_screensaver_ = true;
    device_analog_sense_[0] = 50;
    device_analog_sense_[1] = 50;
  }

private:
  // General Setting
  //   Graphics
  s8    graph_display_device_[32];
  //   Misc
  bool  disable_screensaver_;
  //   Control
  s32   device_analog_sense_[2];
};
#undef kThisVersionClass
#undef kPreviousVersionClass

// -----------------------------------------------------------------------------
// Version 1.0.0.1
#define kThisVersionClass     SettingFileData1001
#define kPreviousVersionClass SettingFileData1000
class kThisVersionClass : public kPreviousVersionClass {
  friend class ggscfg_GeneralSheet;
  friend class ggscfg_NetworkSheet;
  friend class ggscfg_EditCommentDialog;

public:
  enum { kThisVersion = 0x1001 };

  void init() {
    kPreviousVersionClass::init();
    partial_init();
  }

  void upgrade(u16 p_fmt, void* p_data) {
    // さらに前のバージョンが存在する場合は先に適用する
    if (p_fmt <= kPreviousVersionClass::kThisVersion) {
      kPreviousVersionClass::upgrade(p_fmt, p_data);
      partial_init();
    } else {
      memcpy((s8*)this + data_offset(), p_data,
             sizeof(kThisVersionClass) - data_offset());
    }
  }

// accessor/mutator
public:
  bool input_in_background() { return input_in_background_; }
  bool graph_keep_aspect_ratio() { return graph_keep_aspect_ratio_; }

private:
  void partial_init() {
    format_version_ = kThisVersion;
    graph_keep_aspect_ratio_ = true;
    input_in_background_ = false;
  }

private:
  // General Setting
  //   Graphics
  bool  graph_keep_aspect_ratio_;
  //   Misc
  bool  input_in_background_;
};
#undef kThisVersionClass
#undef kPreviousVersionClass

// -----------------------------------------------------------------------------
// Current Version
typedef SettingFileData1001 SettingFileDataCurrent;

#pragma pack(pop)

class SettingFileMgr {
public:
  SettingFileMgr() {
    m_data = NULL;
    m_need_backup = false;
    m_file_format_version = SettingFileDataCurrent::kThisVersion;
  }
  
  ~SettingFileMgr() {
    if (m_data) {
      delete m_data;
      m_data = NULL;
    }
  }

  void read(char* p_filename);
  void write(char* p_filename);

  inline SettingFileDataCurrent* get_data() { return m_data; }

private:
  SettingFileDataCurrent* m_data;
  
  bool  m_need_backup;
  u16   m_file_format_version;
};

#endif // _ggsext_setting
