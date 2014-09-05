/*---------*/
/* include */
/*---------*/
#include <wx/wxprec.h>

#include "ggscfg.h"
#include "ggscfg_ConfigDialog.h"
#include "ggscfg_GeneralSheet.h"
#include "ggscfg_NetworkSheet.h"
#include "ggscfg_ColorEditSheet.h"

#include "device/setting.h"

/*----------*/
/* function */
/*----------*/
ggscfg_ConfigDialog::ggscfg_ConfigDialog(wxWindow* parent) :
    ConfigDialog(parent) {
  m_panel_general->InitDialog();
  m_panel_network->InitDialog();
  m_panel_color_edit->InitDialog();
}

ggscfg_ConfigDialog::~ggscfg_ConfigDialog() {
}

void ggscfg_ConfigDialog::on_ok(wxCommandEvent& event) {
  
  if (m_panel_general->on_dialog_ok(event) == false) {
    return;
  }
  if (m_panel_network->on_dialog_ok(event) == false) {
    return;
  }
  if (m_panel_color_edit->on_dialog_ok(event) == false) {
    return;
  }

  if (value_check()) {
    s8 str[1024];
    sprintf(str, "data/%s/%s", GET_APP->get_title_label(), kSettingFileName);
    g_app.setting_file_mgr()->write(str);
    Close();
  }
}

void ggscfg_ConfigDialog::on_cancel(wxCommandEvent& event) {
  if (m_panel_general->on_dialog_cancel(event) == false) {
    return;
  }
  if (m_panel_network->on_dialog_cancel(event) == false) {
    return;
  }
  if (m_panel_color_edit->on_dialog_cancel(event) == false) {
    return;
  }

  Close();
}

bool ggscfg_ConfigDialog::value_check(void) {
  return true;
}
