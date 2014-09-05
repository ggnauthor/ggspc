/*------*/
/* libs */
/*------*/

/*---------*/
/* include */
/*---------*/
#include "wx/wxprec.h"

#include "ggscfg_ConfigDialog.h"
#include "ggscfg_GeneralSheet.h"
#include "device/input.h"
#include "device/setting.h"

/*----------*/
/* function */
/*----------*/
ggscfg_GeneralSheet::ggscfg_GeneralSheet(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: GeneralSheet(parent, id, pos, size, style)
{
  m_input_mgr = new InputMgr();
  m_input_mgr->init(GetModuleHandle(NULL));

  m_keycfg_target = NULL;

  // 随所で使用するのでコンフィグ用コントロールポインタリストを作成しておく
  wxTextCtrl* tmp[2][16] = {
    m_textCtrlUp_1P,     m_textCtrlDown_1P,  m_textCtrlLeft_1P,     m_textCtrlRight_1P,
    m_textCtrlCircle_1P, m_textCtrlCross_1P, m_textCtrlTriangle_1P, m_textCtrlSquare_1P,
    m_textCtrlL1_1P,     m_textCtrlR1_1P,    m_textCtrlL2_1P,       m_textCtrlR2_1P,
    m_textCtrlL3_1P,     m_textCtrlR3_1P,    m_textCtrlStart_1P,    m_textCtrlSelect_1P,
    m_textCtrlUp_2P,     m_textCtrlDown_2P,  m_textCtrlLeft_2P,     m_textCtrlRight_2P,
    m_textCtrlCircle_2P, m_textCtrlCross_2P, m_textCtrlTriangle_2P, m_textCtrlSquare_2P,
    m_textCtrlL1_2P,     m_textCtrlR1_2P,    m_textCtrlL2_2P,       m_textCtrlR2_2P,
    m_textCtrlL3_2P,     m_textCtrlR3_2P,    m_textCtrlStart_2P,    m_textCtrlSelect_2P,
  };
  memcpy(m_keycfg_ctrl_list, tmp, sizeof(wxTextCtrl*) * 2 * 16);

  // Connect Events
  this->Connect(wxID_ANY, wxEVT_TIMER, wxTimerEventHandler(ggscfg_GeneralSheet::on_timer));
}

ggscfg_GeneralSheet::~ggscfg_GeneralSheet() {
  delete m_input_mgr;
}

BOOL CALLBACK enum_monitor_callback(HMONITOR p_hmon, HDC p_hdc, LPRECT p_rect, LPARAM p_data) {
  ggscfg_GeneralSheet* sheet = (ggscfg_GeneralSheet*)p_data;
  MONITORINFOEX  minfo;
  memset(&minfo, 0x00, sizeof(MONITORINFOEX));
  minfo.cbSize = sizeof(MONITORINFOEX);
  GetMonitorInfo(p_hmon, &minfo);

  sheet->m_choice_display_device->Append(minfo.szDevice);
  return true;
}

void ggscfg_GeneralSheet::on_init(wxInitDialogEvent& event) {

  SettingFileMgr* filemgr = g_app.setting_file_mgr();

  // Graphic
  EnumDisplayMonitors(NULL, NULL, enum_monitor_callback, (LPARAM)this);

  int find = m_choice_display_device->FindString(filemgr->get_data()->graph_display_device_, true);
  m_choice_display_device->Select(find == -1 ? 0 : find);

  m_choice_display_mode->SetSelection(filemgr->get_data()->graph_fullscreen_);
  set_screen_mode_enable();

  m_choice_color_depth->SetSelection(filemgr->get_data()->graph_color_depth_ == 32 ? 0 : 1);

  m_checkBox_frame_skip->SetValue(filemgr->get_data()->graph_frame_skip_);
  m_checkBox_wait_vsync->SetValue(filemgr->get_data()->graph_wait_vsync_);
  m_checkBox_keep_aspect_ratio->SetValue(filemgr->get_data()->graph_keep_aspect_ratio_);

  // Sound
  m_checkBox_bgm->SetValue(filemgr->get_data()->bgm_enable_);
  m_checkBox_se->SetValue(filemgr->get_data()->se_enable_);
  m_checkBox_voice->SetValue(filemgr->get_data()->voice_enable_);

  wxString tmp;
  tmp.Printf("%d", filemgr->get_data()->bgm_volume_);
  m_textCtrl_bgm->SetValue(tmp);
  tmp.Printf("%d", filemgr->get_data()->se_volume_);
  m_textCtrl_se->SetValue(tmp);
  tmp.Printf("%d", filemgr->get_data()->voice_volume_);
  m_textCtrl_voice->SetValue(tmp);

  m_slider_bgm->SetValue(filemgr->get_data()->bgm_volume_);
  m_slider_se->SetValue(filemgr->get_data()->se_volume_);
  m_slider_voice->SetValue(filemgr->get_data()->voice_volume_);

  m_checkBox_play_background->SetValue(filemgr->get_data()->play_in_background_);

  // Misc
  m_checkBox_display_fps->SetValue(filemgr->get_data()->display_fps_);
  m_checkBox_disable_movie->SetValue(filemgr->get_data()->disable_movie_);
  m_checkBox_input_background->SetValue(filemgr->get_data()->input_in_background_);
  m_checkBox_disable_screen_saver->SetValue(filemgr->get_data()->disable_screensaver_);
  m_checkBox_unlock_all_contents->SetValue(filemgr->get_data()->unlock_all_contents_);

  // configファイルのデバイスリストを更新
  for (int i = 0; i < m_input_mgr->device_count(); i++) {
    bool find = false;
    for (int j = 0; j < filemgr->get_data()->device_count_; j++) {
      if (memcmp(&filemgr->get_data()->device_[j].guid, m_input_mgr->getDeviceGuid(i), sizeof(GUID)) == 0) {
        find = true;
        break;
      }
    }
    // 見つからない＆領域に余裕があるなら追加
    if (find == false && filemgr->get_data()->device_count_ < InputMgr::kMaxDeviceCount) {
      memcpy(&filemgr->get_data()->device_[filemgr->get_data()->device_count_].guid, m_input_mgr->getDeviceGuid(i), sizeof(GUID));
      // できればそれなりに動くようにすべき？
      memset(filemgr->get_data()->device_[filemgr->get_data()->device_count_].key, 0xff, sizeof(u32) * 2 * 16);
      filemgr->get_data()->device_count_++;
    }
  }

  // コントロール1Pコンボの初期化
  m_choice_device1P->Append("未使用");
  for (int i = 0; i < m_input_mgr->device_count(); i++) {
    m_choice_device1P->Append(m_input_mgr->getDeviceName(i));
  }
  m_choice_device1P->Select(filemgr->get_data()->select_device_[0]);

  // コントロール2Pコンボの初期化
  m_choice_device2P->Append("未使用");
  for (int i = 0; i < m_input_mgr->device_count(); i++) {
    m_choice_device2P->Append(m_input_mgr->getDeviceName(i));
  }
  m_choice_device2P->Select(filemgr->get_data()->select_device_[1]);

  // アナログ感度スピンコントロールの初期化
  m_spin_analog_sense_1P->SetValue(filemgr->get_data()->device_analog_sense_[0]);
  m_spin_analog_sense_2P->SetValue(filemgr->get_data()->device_analog_sense_[1]);

  update_keyctrls();

  // 20msに１回入力取得
  m_timer.SetOwner(this, wxID_ANY);
  m_timer.Start(20, wxTIMER_CONTINUOUS);
}

void ggscfg_GeneralSheet::on_choice_device(wxCommandEvent& event) {
  // 選択コントロールをリセット
  m_keycfg_target_side = -1;
  m_keycfg_target_idx = -1;
  m_keycfg_target = NULL;

  update_keyctrls();
}

void ggscfg_GeneralSheet::on_left_down(wxMouseEvent& event) {
  
  wxObject* obj = event.GetEventObject();

  // 前回選択していたものを戻す
  if (m_keycfg_target && m_keycfg_target != obj) {
    m_keycfg_target_side = -1;
    m_keycfg_target_idx = -1;
    m_keycfg_target = NULL;
  }

  bool key_edit = false;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      if (obj != m_keycfg_ctrl_list[i][j]) {
        continue;
      }  
      // フォーカスをダミーにセット
      m_textCtrl_dummy->SetFocus();

      if (get_choice_device_ctrl(i)->GetSelection() > 0) {
        // 有効なデバイス
        m_keycfg_target_side = i;
        m_keycfg_target_idx = j;
        m_keycfg_target = (wxTextCtrl*)obj;
      }
      // マッチングループ終了
      key_edit = true;
      break;
    }
    if (key_edit) break;
  }
  // 再描画
  update_keyctrls();

  // キーエディット外なら従来のクリック処理を継続する
  if (key_edit == false) {
    event.Skip();
  }
}

void ggscfg_GeneralSheet::on_scroll_volume(wxScrollEvent& event) {
  wxSlider* slider = (wxSlider*)event.GetEventObject();

  wxTextCtrl* text = NULL;
  if (slider == m_slider_bgm) { text = m_textCtrl_bgm; }
  else if (slider == m_slider_se) { text = m_textCtrl_se; }
  else if (slider == m_slider_voice) { text = m_textCtrl_voice; }

  wxString tmp;
  tmp.Printf("%d", slider->GetValue());
  text->SetValue(tmp);
}

void ggscfg_GeneralSheet::on_text_volume(wxCommandEvent& event) {
  wxTextCtrl* text = (wxTextCtrl*)event.GetEventObject();

  wxSlider* slider = NULL;
  if (text == m_textCtrl_bgm) { slider = m_slider_bgm; }
  else if (text == m_textCtrl_se) { slider = m_slider_se; }
  else if (text == m_textCtrl_voice) { slider = m_slider_voice; }

  long tmp;
  text->GetValue().ToLong(&tmp);
  if (tmp > 128) {
    tmp = 128;
    text->SetValue("128");
  }
  if (tmp < 0) {
    tmp = 0;
    text->SetValue("0");
  }
  slider->SetValue(tmp);
}

void ggscfg_GeneralSheet::on_timer(wxTimerEvent& event) {

  if (this->IsShown() == false) {
    return;
  }

  // キーコンフィグの入力待ち
  if (m_keycfg_target) {
    // デバイスから入力を取得
    m_input_mgr->update();

    int dev_idx = get_choice_device_ctrl(m_keycfg_target_side)->GetSelection() - 1;
    if (dev_idx >= 0 && dev_idx < m_input_mgr->device_count()) {
      // 感度を取得
      int sense = (m_keycfg_target_side == 0 ? m_spin_analog_sense_1P : m_spin_analog_sense_2P)->GetValue();

      int input = m_input_mgr->getKeyCode(dev_idx, sense);
      static int old_input = -1;

      // Escapeキーが押されたらバインド解除する
      if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        input = m_input_mgr->getEmptyCode(dev_idx);
      }

      if (input != - 1 && input != old_input) {
        SettingFileMgr* filemgr = g_app.setting_file_mgr();
        filemgr->get_data()->device_[dev_idx].key[m_keycfg_target_side][m_keycfg_target_idx] = input;

        // 自動的に次のコントロールへ移動
        m_keycfg_target_idx++;
        if (m_keycfg_target_idx >= 16) {
          m_keycfg_target_side = -1;
          m_keycfg_target_idx = -1;
          m_keycfg_target = NULL;
        } else {
          m_keycfg_target = m_keycfg_ctrl_list[m_keycfg_target_side][m_keycfg_target_idx];
        }
        update_keyctrls();
      }
      old_input = input;
    }
  }
}

void ggscfg_GeneralSheet::onchoice_screen_mode(wxCommandEvent& event) {
 set_screen_mode_enable();
}

bool ggscfg_GeneralSheet::on_dialog_ok(wxCommandEvent& event) {

  SettingFileMgr* filemgr = g_app.setting_file_mgr();

  // 各種設定を保存
  // Graphics
  filemgr->get_data()->graph_fullscreen_    = m_choice_display_mode->GetSelection();
  filemgr->get_data()->graph_frame_skip_    = m_checkBox_frame_skip->GetValue();
  filemgr->get_data()->graph_wait_vsync_    = m_checkBox_wait_vsync->GetValue();
  strcpy(filemgr->get_data()->graph_display_device_, m_choice_display_device->GetStringSelection());
  filemgr->get_data()->graph_color_depth_   = m_choice_color_depth->GetSelection() == 0 ? 32 : 16;
  filemgr->get_data()->graph_keep_aspect_ratio_ = m_checkBox_keep_aspect_ratio->GetValue();

  // Sound
  filemgr->get_data()->bgm_enable_ = m_checkBox_bgm->GetValue();
  filemgr->get_data()->se_enable_ = m_checkBox_se->GetValue();
  filemgr->get_data()->voice_enable_ = m_checkBox_voice->GetValue();

  filemgr->get_data()->bgm_volume_ = m_slider_bgm->GetValue();
  filemgr->get_data()->se_volume_ = m_slider_se->GetValue();
  filemgr->get_data()->voice_volume_ = m_slider_voice->GetValue();

  filemgr->get_data()->play_in_background_ = m_checkBox_play_background->GetValue();

  // Misc
  filemgr->get_data()->display_fps_ = m_checkBox_display_fps->GetValue();
  filemgr->get_data()->disable_movie_ = m_checkBox_disable_movie->GetValue();
  filemgr->get_data()->disable_screensaver_ = m_checkBox_disable_screen_saver->GetValue();
  filemgr->get_data()->input_in_background_ = m_checkBox_input_background->GetValue();
  filemgr->get_data()->unlock_all_contents_ = m_checkBox_unlock_all_contents->GetValue();

  // Controls
  filemgr->get_data()->select_device_[0] = m_choice_device1P->GetSelection();
  filemgr->get_data()->select_device_[1] = m_choice_device2P->GetSelection();

  filemgr->get_data()->device_analog_sense_[0] = m_spin_analog_sense_1P->GetValue();
  if (filemgr->get_data()->device_analog_sense_[0] > 100) {
    filemgr->get_data()->device_analog_sense_[0] = 100;
  } else if (filemgr->get_data()->device_analog_sense_[0] < 0) {
    filemgr->get_data()->device_analog_sense_[0] = 0;
  }

  filemgr->get_data()->device_analog_sense_[1] = m_spin_analog_sense_2P->GetValue();
  if (filemgr->get_data()->device_analog_sense_[1] > 100) {
    filemgr->get_data()->device_analog_sense_[1] = 100;
  } else if (filemgr->get_data()->device_analog_sense_[1] < 0) {
    filemgr->get_data()->device_analog_sense_[1] = 0;
  }

  return true;
}

bool ggscfg_GeneralSheet::on_dialog_cancel(wxCommandEvent& event) {
  return true;
}

void ggscfg_GeneralSheet::update_keyctrls(void) {

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      wxColour color;
      if (m_keycfg_ctrl_list[i][j] == m_keycfg_target) {
        color.Set(kKeyCfgCtrlHighlightColor);
      } else if (get_choice_device_ctrl(i)->GetSelection() == 0) {
        color.Set(kKeyCfgCtrlDisableColor);
      } else {
        color.Set(kKeyCfgCtrlDefaultColor);
      }
      m_keycfg_ctrl_list[i][j]->SetBackgroundColour(color);
      m_keycfg_ctrl_list[i][j]->Refresh();
    }
  }

  for (int i = 0; i < 2; i++) {
    SettingFileMgr* filemgr = g_app.setting_file_mgr();
    
    int dev_idx = get_choice_device_ctrl(i)->GetSelection() - 1;
    if (dev_idx == 0) {
      // キーボード
      for (int j = 0; j < 16; j++) {
        int key = filemgr->get_data()->device_[dev_idx].key[i][j];
        m_keycfg_ctrl_list[i][j]->SetValue(m_input_mgr->key2str(key));
      }
    } else if (dev_idx > 0) {
      // パッド
      for (int j = 0; j < 16; j++) {
        int key = filemgr->get_data()->device_[dev_idx].key[i][j];
        m_keycfg_ctrl_list[i][j]->SetValue(m_input_mgr->joy2str(key));
      }
    } else {
      // 未使用
      for (int j = 0; j < 16; j++) {
        m_keycfg_ctrl_list[i][j]->SetValue("");
      }
    }
  }
}

void ggscfg_GeneralSheet::set_screen_mode_enable() {
  if (m_choice_display_mode->GetSelection() ==
        SettingFileData::kDisplayMode_FullScreenExcl) {
    m_checkBox_keep_aspect_ratio->Disable();
  } else {
    m_checkBox_keep_aspect_ratio->Enable();
  }
}
