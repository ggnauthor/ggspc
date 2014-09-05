#include "main.h"
#include "setting.h"

#include "utils/ds_zlib.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

namespace {

const s8  kDefaultLobbyFont[] = "ＭＳ Ｐゴシック";
const s8  kDefaultPlayerName[] = "No Name";
const s8  kDefaultTripKey[] = "";
const s8  kDefaultComment[] = "No Comment";
const int kDefaultLobbyFontSize = 12;

};

void SettingFileData::init() {
  format_version_ = 0x0000;
  
  // General Setting
  //   Graphics
  graph_fullscreen_  = false;
  graph_frame_skip_  = false;
  graph_wait_vsync_  = false;
  graph_color_depth_ = 32;
  //   Sound
  bgm_enable_   = true;
  bgm_volume_   = 128;
  se_enable_    = true;
  se_volume_    = 64;
  voice_enable_ = true;
  voice_volume_ = 96;
  play_in_background_ = true;
  //   Misc
  unlock_all_contents_ = false;
  display_fps_   = false;
  disable_movie_ = true;
  //   Control
  device_count_ = 0;
  select_device_[0] = 1;  // キーボードは存在する前提でよい
  select_device_[1] = 0;
  for (int i = 0; i < InputMgr::kMaxDeviceCount; i++) {
    memset(&device_[i].guid, 0, sizeof(GUID));
    memset(&device_[i].key[0], 0, sizeof(u32) * 16);
    memset(&device_[i].key[1], 0, sizeof(u32) * 16);
  }
  // NetworkSetting
  //   Connection
  port_ = rand() % (65535 - 1024) + 1024;
  delay_frame_    = 0;
  rollback_frame_ = 2;
  //   Rule
  enable_ex_   = false;
  enable_sp_   = false;
  game_rounds_ = 5;
  //   Watch
  allow_watch_       = true;
  allow_intrusion_   = false;
  save_watch_replay_ = false;
  max_relay_node_count_ = 1;
  //   Lobby
  lobby_select_script_ = 0;
  lobby_url_count_     = 0;
  for (int i = 0; i < 8; i++) {
    strcpy(lobby_url_[i], "");
  }
  strcpy(lobby_font_face_, kDefaultLobbyFont);
  lobby_font_size_ = kDefaultLobbyFontSize;
  enable_lobby_font_antialias_ = false;
  //   Deny
  deny_method_       = kDenyMethod_ClientCode;
  deny_slow_user_    = true;
  deny_disconnecter_ = true;
  //   Profile
  strcpy(player_name_, kDefaultPlayerName);
  strcpy(trip_key_, kDefaultTripKey);
  strcpy(comment_, kDefaultComment);
  rank_       = kRankF;
  win_count_  = 0;
  game_count_ = 0;
  win_rate_   = 0.0f;
  slow_rate_  = 0.0f;
  disconnect_rate_ = 0.0f;
}

void SettingFileMgr::read(char* p_filename) {
  if (m_data) {
    delete m_data;
    m_data = NULL;
  }
  FILE *fp = fopen(p_filename, "rb");
  if (fp) {
    int unz_size = ds_zlib::zfsize(fp);
    s8* buf = (s8*)malloc(unz_size);

    ds_zlib::zfread(buf, unz_size, fp);
    m_file_format_version = ((u16*)buf)[0];

    if (m_file_format_version <= SettingFileDataCurrent::kThisVersion) {
      m_data = new SettingFileDataCurrent();
      if (m_file_format_version == SettingFileDataCurrent::kThisVersion) {
        // バージョン変化なし
        memcpy((s8*)m_data + m_data->data_offset(), buf,
               sizeof(SettingFileDataCurrent) - m_data->data_offset());
        m_need_backup = false;
      } else {
        // 必要分のアップグレードを適用する
        m_data->upgrade(m_file_format_version, (SettingFileData*)buf);
        m_need_backup = true;
      }
    } else {
      // ダウングレードは完全新規。
      // 書き込み時にバックアップはとるが設定は読み込まれない
      m_data = new SettingFileDataCurrent();
      m_data->init();
      m_need_backup = true;
    }
    free(buf);
    fclose(fp);
  } else {
    // ファイルがない場合、最新バージョンとして作成
    m_data = new SettingFileDataCurrent();
    m_data->init();
    m_need_backup = false;
  }
}

void SettingFileMgr::write(char* p_filename) {
  // アップグレードしている場合、
  // 書き込み前に前のバージョンのファイルを保存する
  if (m_need_backup) {
    char str[1024];
    sprintf(str, "%s%03x", p_filename,
      (m_file_format_version >> 4) & 0x0fff);

    if ((m_file_format_version & 0x0f) >= 0x0a) {
      s8 tmp[8];
      sprintf(tmp, "%x", m_file_format_version & 0x0f);
      strcat(str, tmp);
    }
    remove(str);
    rename(p_filename, str);
  }

  FILE *fp = fopen(p_filename, "wb");
  if (fp) {
    ds_zlib::zfwrite(m_data->get_data(),
                     sizeof(SettingFileDataCurrent) - 4, fp);
    fclose(fp);
  }
}
