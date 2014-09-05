/*---------*/
/* include */
/*---------*/
#include "wx/wxprec.h"
#include "wx/fontenum.h"

#include "ggscfg_NetworkSheet.h"
#include "ggscfg_EditCommentDialog.h"

#include "device/setting.h"

/*----------*/
/* function */
/*----------*/
ggscfg_NetworkSheet::ggscfg_NetworkSheet(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: NetworkSheet(parent, id, pos, size, style)
{
}

ggscfg_NetworkSheet::~ggscfg_NetworkSheet() {
}

void ggscfg_NetworkSheet::on_init(wxInitDialogEvent& event) {
  SettingFileMgr* filemgr = g_app.setting_file_mgr();

  // Connection
  wxString tmp;
  tmp.Printf("%d", filemgr->get_data()->port_);
  m_textCtrl_port->SetValue(tmp);
  
  m_choice_delay->SetSelection(filemgr->get_data()->delay_frame_);
  m_choice_rollback->SetSelection(filemgr->get_data()->rollback_frame_);

  // Rule
  m_checkBox_ex->SetValue(filemgr->get_data()->enable_ex_);
  m_checkBox_cheat->SetValue(filemgr->get_data()->enable_sp_);
  m_choice_round->SetSelection(filemgr->get_data()->game_rounds_);

  // Watch
  m_checkBox_allow_watch->SetValue(filemgr->get_data()->allow_watch_);
  m_checkBox_allow_intrusion->SetValue(filemgr->get_data()->allow_intrusion_);
  m_checkBox_save_replay->SetValue(filemgr->get_data()->save_watch_replay_);
  m_choice_max_relay->SetSelection(filemgr->get_data()->max_relay_node_count_);

  // Lobby
  for (int i = 0; i < filemgr->get_data()->lobby_url_count_; i++) {
    m_comboBox_lobby_address->Append(filemgr->get_data()->lobby_url_[i]);
  }
  m_comboBox_lobby_address->SetSelection(filemgr->get_data()->lobby_select_script_);
  
  wxFontEnumerator font_enum;
  font_enum.EnumerateFacenames(wxFONTENCODING_CP932, false);
  wxArrayString fontlist = font_enum.GetFacenames();
  
  m_comboBox_font_face->Freeze();
  m_comboBox_font_face->Append(fontlist);
  for (u32 i = 0; i < fontlist.GetCount(); i++) {
    if (strcmp(filemgr->get_data()->lobby_font_face_, fontlist[i]) == 0) {
      m_comboBox_font_face->SetSelection(i);
      break;
    }
  }
  m_comboBox_font_face->Thaw();

  tmp.Printf("%d", filemgr->get_data()->lobby_font_size_);
  m_textCtrl_font_size->SetValue(tmp);

  m_checkBox_antialias->SetValue(filemgr->get_data()->enable_lobby_font_antialias_);

  // Deny
  m_choice_deny_type->SetSelection(filemgr->get_data()->deny_method_);
  m_checkBox_deny_slow->SetValue(filemgr->get_data()->deny_slow_user_);
  m_checkBox_deny_disconnect->SetValue(filemgr->get_data()->deny_disconnecter_);

  // Profile
  m_textCtrl_player_name->SetValue(filemgr->get_data()->player_name_);
  m_textCtrl_tripkey->SetValue(filemgr->get_data()->trip_key_);
  {
    wxString tmp;
    
    tmp.Printf("%c", kRankChars[filemgr->get_data()->rank_]);
    m_textCtrl_rank->SetValue(tmp);
    
    tmp.Printf("%d", filemgr->get_data()->win_count_);
    m_textCtrl_win_count->SetValue(tmp);
    
    tmp.Printf("%d", filemgr->get_data()->game_count_);
    m_textCtrl_game_count->SetValue(tmp);
    
    tmp.Printf("%.2f", filemgr->get_data()->win_rate_);
    m_textCtrl_win_rate->SetValue(tmp);
    
    tmp.Printf("%.2f", filemgr->get_data()->slow_rate_);
    m_textCtrl_slow_rate->SetValue(tmp);
    
    tmp.Printf("%.2f", filemgr->get_data()->disconnect_rate_);
    m_textCtrl_disconnect_rate->SetValue(tmp);
  }
}

void ggscfg_NetworkSheet::on_button_clicked_lobby_address_add(wxCommandEvent& event) {
  SettingFileMgr* filemgr = g_app.setting_file_mgr();

  if (m_comboBox_lobby_address->GetValue().GetData()[0] != '\0'
  &&  filemgr->get_data()->lobby_url_count_ < SettingFileDataCurrent::kMaxLobbyAddressCount) {
    
    bool find = false;
    for (int i = 0; i < filemgr->get_data()->lobby_url_count_; i++) {
      if (strcmp(filemgr->get_data()->lobby_url_[i], m_comboBox_lobby_address->GetValue().GetData()) == 0) {
        find = true;
      }
    }
    if (find == false) {
      strcpy(
        filemgr->get_data()->lobby_url_[filemgr->get_data()->lobby_url_count_],
        m_comboBox_lobby_address->GetValue().GetData());
      m_comboBox_lobby_address->Append(m_comboBox_lobby_address->GetValue());
      m_comboBox_lobby_address->SetSelection(filemgr->get_data()->lobby_url_count_);
      filemgr->get_data()->lobby_url_count_++;
    }
  }
}

void ggscfg_NetworkSheet::on_button_clicked_lobby_address_del(wxCommandEvent& event) {

  SettingFileMgr* filemgr = g_app.setting_file_mgr();

  if (filemgr->get_data()->lobby_url_count_ > 0) {
    
    int sel = m_comboBox_lobby_address->GetSelection();
    int end = filemgr->get_data()->lobby_url_count_ - 1;
    for (int i = sel; i < end; i++) {
      strcpy(filemgr->get_data()->lobby_url_[i], filemgr->get_data()->lobby_url_[i + 1]);
    }
    filemgr->get_data()->lobby_url_[end][0] = '\0';
    filemgr->get_data()->lobby_url_count_--;

    m_comboBox_lobby_address->Delete(sel);
    m_comboBox_lobby_address->SetSelection(sel == 0 ? 0 : sel - 1);
  }
}

void ggscfg_NetworkSheet::on_button_clicked_edit_comment(wxCommandEvent& event) {
  wxPoint pt = wxGetMousePosition();
  ggscfg_EditCommentDialog dialog(this);
  dialog.InitDialog();
  dialog.SetPosition(wxPoint(pt.x - 150, pt.y - 150));
  dialog.ShowModal();
}

bool ggscfg_NetworkSheet::on_dialog_ok(wxCommandEvent& event) {
  SettingFileMgr* filemgr = g_app.setting_file_mgr();

  // 各種設定を保存
  // Connection
  long tmp;
  m_textCtrl_port->GetValue().ToLong(&tmp);
  filemgr->get_data()->port_ = tmp;

  filemgr->get_data()->delay_frame_ = m_choice_delay->GetSelection();
  filemgr->get_data()->rollback_frame_ = m_choice_rollback->GetSelection();

  // Rule
  filemgr->get_data()->enable_ex_ = m_checkBox_ex->GetValue();
  filemgr->get_data()->enable_sp_ = m_checkBox_cheat->GetValue();
  filemgr->get_data()->game_rounds_ = m_choice_round->GetSelection();

  // Watch
  filemgr->get_data()->allow_watch_ = m_checkBox_allow_watch->GetValue();
  filemgr->get_data()->allow_intrusion_ = m_checkBox_allow_intrusion->GetValue();
  filemgr->get_data()->save_watch_replay_ = m_checkBox_save_replay->GetValue();
  filemgr->get_data()->max_relay_node_count_ = m_choice_max_relay->GetSelection();
  
  // Lobby
  wxCommandEvent ev;
  on_button_clicked_lobby_address_add(ev);
  filemgr->get_data()->lobby_select_script_ = m_comboBox_lobby_address->GetSelection();
  strcpy(filemgr->get_data()->lobby_font_face_, m_comboBox_font_face->GetValue().GetData());
  m_textCtrl_font_size->GetValue().ToLong(&tmp);
  filemgr->get_data()->lobby_font_size_ = tmp;
  filemgr->get_data()->enable_lobby_font_antialias_ = m_checkBox_antialias->GetValue();

  // Deny
  filemgr->get_data()->deny_method_ = m_choice_deny_type->GetSelection();
  filemgr->get_data()->deny_slow_user_ = m_checkBox_deny_slow->GetValue();
  filemgr->get_data()->deny_disconnecter_ = m_checkBox_deny_disconnect->GetValue();

  // Profile
  strncpy(filemgr->get_data()->player_name_, m_textCtrl_player_name->GetValue().GetData(), SettingFileDataCurrent::kPlayerNameSize);
  strncpy(filemgr->get_data()->trip_key_, m_textCtrl_tripkey->GetValue(), SettingFileDataCurrent::kTripKeySize);
  
  return true;
}

bool ggscfg_NetworkSheet::on_dialog_cancel(wxCommandEvent& event) {
  return true;
}
