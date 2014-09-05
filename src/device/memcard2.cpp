#include "device/memcard2.h"

#include "main.h"
#include "memory/memory.h"
#include "cpu/r5900.h"
#if defined _DEBUGGER
#include "debugger/debugger.h"
#endif
#include <shlwapi.h> // for PathIsDirectory
#pragma comment(lib, "shlwapi.lib")
#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

MemCard2::MemCard2() {
  for (int i = 0; i < 2; i++) {
    get_info_last_state_[i].type = 0;
    get_info_last_state_[i].free = 0;
    get_info_last_state_[i].format = 0;
  }
  relative_dir_[0] = '\0';
}

MemCard2::~MemCard2() {
  clearSocket();
}

void MemCard2::clearSocket() {
  for (vector<Mc2SocketInfo*>::iterator itr = socket_.begin();
       itr != socket_.end();
       ++itr) {
    delete *itr;
  }
  socket_.clear();
}

void MemCard2::callMemCard2Rpc(u32 p_client_data_ptr, u32 p_fno,
                               u32 p_send_buf_ptr, u32 p_send_buf_size,
                               u32 p_recv_buf_ptr, u32 p_recv_buf_size) {
  u32* clidata = (u32*)VA2PA(p_client_data_ptr);
  u32* sendbuf = (u32*)VA2PA(p_send_buf_ptr);
  s32* recvbuf = (s32*)VA2PA(p_recv_buf_ptr);

  switch (p_fno) {
    case 0x80001301: { // dbc create socket
      Mc2SocketParam* param  = (Mc2SocketParam*)sendbuf;
      Mc2SocketInfo* mc2sock = new Mc2SocketInfo;
      mc2sock->port = param->port - 2;
      mc2sock->slot = param->slot;
      socket_.push_back(mc2sock);
      g_cpu->m_v0.d[0] = socket_.size();
      DBGOUT_CPU("Dbc create succeeded! sockno=%d option=%08x "
        "port=%d slot=%d name=%s\n", g_cpu->m_v0.d[0],
        param->option, param->port & 1, param->slot, param->name);
    } break;
    case 0x80001363: { // dbc init
      relative_dir_[0] = '\0';
      clearSocket();
      recvbuf[0] = 0x00000320;
      DBGOUT_CPU("Dbc init succeeded!\n");
    } break;
    case 0x8000131c: { // dbc send
      DBGOUT_CPU("Dbc send not supported!\n");
    } break;
    default:           // invalid
      DBGOUT_CPU("SifCallRpc Dbc(MemCd2) Invalid FunctionID!!\n");
      break;
  }
}

void MemCard2::getinfo(u32 p_socket, u32 p_buf_va) {
  u32* buf  = (u32*)VA2PA(p_buf_va);
  u32  port = socket_[p_socket]->port;
  u32  slot = socket_[p_socket]->slot;
  assert(port < 2);
  assert(slot == 0);

  // メモリーカードが刺さっていない状況は用意しない
  // デバッグ等でどうしても必要な場合は以下のフラグをfalseにすること
  bool exist     = true;
  int  formatted = false;

  // memcard1ディレクトリが存在する場合、フォーマット済みとみなす
  s8 mcd_path[1024];
  sprintf(mcd_path, "%s/memcard%d", g_base_dir, port + 1);
  if (PathIsDirectory(mcd_path)) {
    formatted = true;
  }
  s32 result;
  u32 type;
  u32 free_size;
  if (exist && formatted) {
    // フォーマット済み
    result = 0x81019003;
    type = 2;
    free_size = getDiskFree();
  } else if (exist && formatted == false) {
    // 未フォーマット
    result = 0x8101002f;
    type = 2;
    free_size = 0;
  } else {
    // 未接続
    result = 0x81010013;
    type = 0;
    free_size = 0;
  }

  buf[0] = type;
  buf[2] = free_size;
  buf[1] = formatted;

  // 前回と同じなら result = 0 とする
  // これはMcSyncにて取得される
  if (get_info_last_state_[port].type   == type &&
      get_info_last_state_[port].free   == free_size &&
      get_info_last_state_[port].format == formatted) {
    result = 0;
  } else {
    // 現在の状態を保存する
    get_info_last_state_[port].type   = type;
    get_info_last_state_[port].free   = free_size;
    get_info_last_state_[port].format = formatted;
  }

  g_cpu->m_v0.d[0] = result;
  DBGOUT_CPU("%s succeeded! port=%d slot=%d mcd_path=%s result=%d\n",
    __FUNCTION__, port, slot, mcd_path, result);
}

void MemCard2::read(u32 p_socket, u32 p_name_va, u32 p_buf_va, u32 p_ofs, u32 p_size) {
  u32* buf  = (u32*)VA2PA(p_buf_va + p_ofs);
  s8*  name = (s8*)VA2PA(p_name_va);
  u32  port = socket_[p_socket]->port;
  u32  slot = socket_[p_socket]->slot;
  assert(port < 2);
  assert(slot == 0);

  s8 path[1024];
  if (strchr(name, '/')) {
    // フルパスと見なす
    sprintf(path, "%s/memcard%d/%s",
      g_base_dir, slot + 1, name);
  } else {
    sprintf(path, "%s/memcard%d/%s/%s",
      g_base_dir, slot + 1, relative_dir_, name);
  }
  FILE* fp = fopen(path, "rb");
  if (fp) {
    size_t actual_size = fread(buf, 1, p_size, fp);
    fclose(fp);
    g_cpu->m_v0.d[0] = actual_size;
  } else {
    g_cpu->m_v0.d[0] = -1;
  }
  DBGOUT_CPU("%s succeeded! socket=%d, buf=0x%08x, size=%d\n",
    __FUNCTION__, p_socket, p_buf_va, p_size);
}

void MemCard2::write(u32 p_socket, u32 p_name_va, u32 p_buf_va, u32 p_ofs, u32 p_size) {
  u32* buf = (u32*)VA2PA(p_buf_va + p_ofs);
  s8*  name = (s8*)VA2PA(p_name_va);
  u32  port = socket_[p_socket]->port;
  u32  slot = socket_[p_socket]->slot;
  assert(port < 2);
  assert(slot == 0);

  s8 path[1024];
  if (strchr(name, '/')) {
    // フルパスと見なす
    sprintf(path, "%s/memcard%d/%s",
      g_base_dir, slot + 1, name);
  } else {
    sprintf(path, "%s/memcard%d/%s/%s",
      g_base_dir, slot + 1, relative_dir_, name);
  }
  FILE* fp = fopen(path, "wb");
  if (fp) {
    size_t actual_size = fwrite(buf, 1, p_size, fp);
    fclose(fp);
    g_cpu->m_v0.d[0] = actual_size;
  } else {
    g_cpu->m_v0.d[0] = -1;
  }
  DBGOUT_CPU("%s succeeded! socket=%d buf=0x%08x, size=%d\n",
    __FUNCTION__, p_socket, p_buf_va, p_size);
}

void MemCard2::mkdir(u32 p_socket, u32 p_name_va) {
  s8*    name = (s8*)VA2PA(p_name_va);
  u32    port = socket_[p_socket]->port;
  u32    slot = socket_[p_socket]->slot;
  assert(port < 2);
  assert(slot == 0);

  s8 path[1024];
  if (strchr(name, '/')) {
    // フルパスと見なす
    sprintf(path, "%s/memcard%d/%s",
      g_base_dir, slot + 1, name);
  } else {
    sprintf(path, "%s/memcard%d/%s/%s",
      g_base_dir, slot + 1, relative_dir_, name);
  }
  u32 result = 0x00000000;
  if (!CreateDirectory((LPCSTR)path, NULL)) {
    result = 0x81010000;
  }
  
  g_cpu->m_v0.d[0] = result;
  DBGOUT_CPU("%s succeeded! port=%d, name=%s\n", __FUNCTION__, port, name);
}

void MemCard2::getdir(u32 p_socket, u32 p_name_va, u32 p_ofs,
                      u32 p_maxent, u32 p_buf_va, u32 p_cnt_va) {
  s8*  name  = (s8*)VA2PA(p_name_va);
  u32* buf   = (u32*)VA2PA(p_buf_va);
  u32* count = (u32*)VA2PA(p_cnt_va);
  u32  port  = socket_[p_socket]->port;
  u32  slot  = socket_[p_socket]->slot;
  assert(port < 2);
  assert(slot == 0);

  s8 file_path[1024];
  sprintf(file_path, "%s/memcard%d/%s", g_base_dir, port + 1, name);

  s32 result = 0x00000000;
  Mc2GetDirEntry* ent_table = (Mc2GetDirEntry*)buf;
  WIN32_FIND_DATA  find_data;
  HANDLE hdl = FindFirstFile(file_path, &find_data);
  if (hdl == INVALID_HANDLE_VALUE) {
    result = 0x81010002;
  } else {
    BOOL succeeded;
    u32 file_cnt = 0;
    u32 ent_cnt = 0;
    do {
      if (file_cnt >= p_ofs) {
        strncpy(ent_table[ent_cnt].name, find_data.cFileName, 31);
        ent_table[ent_cnt].name[31] = '\0';

        ent_table[ent_cnt].size = find_data.nFileSizeLow;
        ent_table[ent_cnt].attr = 0x0087 |
          ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ?
             0x0020 : 0x0000);

        SYSTEMTIME systime;
        FileTimeToSystemTime(&find_data.ftCreationTime, &systime);
        ent_table[ent_cnt].create.year  = (u16)systime.wYear;
        ent_table[ent_cnt].create.month = (u8)systime.wMonth;
        ent_table[ent_cnt].create.day   = (u8)systime.wDay;
        ent_table[ent_cnt].create.hour  = (u8)systime.wHour;
        ent_table[ent_cnt].create.min   = (u8)systime.wMinute;
        ent_table[ent_cnt].create.sec   = (u8)systime.wSecond;

        FileTimeToSystemTime(&find_data.ftLastWriteTime, &systime);
        ent_table[ent_cnt].modify.year  = (u16)systime.wYear;
        ent_table[ent_cnt].modify.month = (u8)systime.wMonth;
        ent_table[ent_cnt].modify.day   = (u8)systime.wDay;
        ent_table[ent_cnt].modify.hour  = (u8)systime.wHour;
        ent_table[ent_cnt].modify.min   = (u8)systime.wMinute;
        ent_table[ent_cnt].modify.sec   = (u8)systime.wSecond;
        ent_cnt++;
      }
      file_cnt++;
      succeeded = FindNextFile(hdl, &find_data);
    } while (succeeded && ent_cnt < p_maxent);
    FindClose(hdl);
    *count = ent_cnt;
  }
  g_cpu->m_v0.d[0] = result;
  DBGOUT_CPU("%s succeeded! port=%d slot=%d ofs=%d maxent=%d "
    "name=%s result=%d\n", __FUNCTION__, port, slot,
    p_ofs, p_maxent, name, result);
}

void MemCard2::chdir(u32 p_socket, u32 p_name_va, u32 p_pwd_va) {
  s8*    name = (s8*)VA2PA(p_name_va);
  u32    port = socket_[p_socket]->port;
  u32    slot = socket_[p_socket]->slot;
  assert(port < 2);
  assert(slot == 0);

  if (p_pwd_va) {
    s8* pwd = (s8*)VA2PA(p_pwd_va);
    strcpy(pwd, relative_dir_);
  }

  strcpy(relative_dir_, name);

  g_cpu->m_v0.d[0] = 0x00000000;
  DBGOUT_CPU("%s succeeded! port=%d slot=%d path=%s\n",
    __FUNCTION__, port, slot, name);
}

void MemCard2::format(u32 p_socket) {
  u32    slot = socket_[p_socket]->slot;
  assert(slot == 0);

  s32 result = 0x00000000;

  s8 mcd_path[1024];
  sprintf(mcd_path, "%s/memcard%d", g_base_dir, slot + 1);
  if (CreateDirectory(mcd_path, NULL)) {
    get_info_last_state_[slot].type   = 2;
    get_info_last_state_[slot].free   = getDiskFree();
    get_info_last_state_[slot].format = 1;
  } else {
    result = -1;
  }
  g_cpu->m_v0.d[0] = result;
  DBGOUT_CPU("%s succeeded! slot=%d result=%d\n", __FUNCTION__, slot, result);
}

u32 MemCard2::getDiskFree() {
  ULARGE_INTEGER use, total, free;
  GetDiskFreeSpaceEx(g_base_dir, &use, &total, &free);
  return free.HighPart > 0 ? 0x7fffffff : free.LowPart;
}
