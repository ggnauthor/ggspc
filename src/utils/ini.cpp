#include "main.h"
#include "ini.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

void IniFileInfo::read() {
  s8 inipath[1024];
  sprintf(inipath, "%s/%s.ini", g_base_dir, APPNAME);

  GetPrivateProfileString(
    "General", "LastOpen", "", last_open_, 1024, inipath);
}

void IniFileInfo::write() {
  s8 inipath[1024];
  sprintf(inipath, "%s/%s.ini", g_base_dir, APPNAME);

  WritePrivateProfileString(
    "General", "LastOpen", last_open_, inipath);
}
