/*---------*/
/* include */
/*---------*/
#include <wx/wxprec.h>

#include "main.h"
#include "ggscfg.h"
#include "ggscfg_gui.h"
#include "ggscfg_ConfigDialog.h"

#include "device/setting.h"

/*--------*/
/* global */
/*--------*/
IMPLEMENT_APP(GgsCfgApp)

/*----------*/
/* function */
/*----------*/
bool GgsCfgApp::OnInit() {

  // G-EMU本体からタイトルラベルが渡されてくる
  if (argc > 1) {
    strcpy(m_title_label, argv[1]);

    g_app.init();

    s8 str[1024];
    sprintf(str, "data/%s/%s", GET_APP->get_title_label(), kSettingFileName);
    g_app.setting_file_mgr()->read(str);

    ggscfg_ConfigDialog dialog(NULL);
    dialog.ShowModal();

    g_app.clean();
  }

  return false;
}

int GgsCfgApp::OnExit() {
  return 0;
}
