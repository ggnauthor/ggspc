#include "main.h"
#include "setting.h"

#include "utils/ds_zlib.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

namespace {

const s8  kDefaultLobbyFont[] = "�l�r �o�S�V�b�N";
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
  select_device_[0] = 1;  // �L�[�{�[�h�͑��݂���O��ł悢
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
        // �o�[�W�����ω��Ȃ�
        memcpy((s8*)m_data + m_data->data_offset(), buf,
               sizeof(SettingFileDataCurrent) - m_data->data_offset());
        m_need_backup = false;
      } else {
        // �K�v���̃A�b�v�O���[�h��K�p����
        m_data->upgrade(m_file_format_version, (SettingFileData*)buf);
        m_need_backup = true;
      }
    } else {
      // �_�E���O���[�h�͊��S�V�K�B
      // �������ݎ��Ƀo�b�N�A�b�v�͂Ƃ邪�ݒ�͓ǂݍ��܂�Ȃ�
      m_data = new SettingFileDataCurrent();
      m_data->init();
      m_need_backup = true;
    }
    free(buf);
    fclose(fp);
  } else {
    // �t�@�C�����Ȃ��ꍇ�A�ŐV�o�[�W�����Ƃ��č쐬
    m_data = new SettingFileDataCurrent();
    m_data->init();
    m_need_backup = false;
  }
}

void SettingFileMgr::write(char* p_filename) {
  // �A�b�v�O���[�h���Ă���ꍇ�A
  // �������ݑO�ɑO�̃o�[�W�����̃t�@�C����ۑ�����
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
