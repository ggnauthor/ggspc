#include "main.h"
#include "device/setting.h"

#include <stdio.h>

App g_app;

void App::init() {
  setting_file_mgr_ = new SettingFileMgr();
}

void App::clean() {
  delete setting_file_mgr_;
}
