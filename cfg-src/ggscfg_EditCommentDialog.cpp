/*---------*/
/* include */
/*---------*/
#include "ggscfg_EditCommentDialog.h"

#include "device/setting.h"

/*----------*/
/* function */
/*----------*/
ggscfg_EditCommentDialog::ggscfg_EditCommentDialog(wxWindow* parent) : EditCommentDialog(parent) {
  SettingFileMgr* filemgr = g_app.setting_file_mgr();

  m_textCtrl_comment->SetValue(filemgr->get_data()->comment_);
}

void ggscfg_EditCommentDialog::on_ok(wxCommandEvent& event) {
  SettingFileMgr* filemgr = g_app.setting_file_mgr();
  strcpy(filemgr->get_data()->comment_, m_textCtrl_comment->GetValue().GetData());
  Close();
}

void ggscfg_EditCommentDialog::on_cancel(wxCommandEvent& event) {
  Close();
}
