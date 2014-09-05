#include "device/memcard.h"

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

MemCard::MemCard() :
    get_dir_count_(0) {
  for (int i = 0; i < 2; i++) {
    get_info_last_state_[i].valid = false;
    get_info_last_state_[i].type = 0;
    get_info_last_state_[i].free = 0;
    get_info_last_state_[i].format = 0;
  }
  relative_dir_[0] = '\0';
}

MemCard::~MemCard() {
  for (u32 i = 0; i < file_mgmt_.size(); i++) {
    closeMcdFile(file_mgmt_[i]->fd);
  }
}

void MemCard::callMemCardRpc(u32 p_client_data_ptr, u32 p_fno,
                             u32 p_send_buf_ptr, u32 p_send_buf_size,
                             u32 p_recv_buf_ptr, u32 p_recv_buf_size) {
  // xmcman/xmcserv

  u32* clidata = (u32*)VA2PA(p_client_data_ptr);
  u32* sendbuf = (u32*)VA2PA(p_send_buf_ptr);
  s32* result  = (s32*)VA2PA(p_recv_buf_ptr);

  switch (p_fno) {
    case 0x01: // get info
      getinfo(clidata, sendbuf, result);
      break;
    case 0x02: // open
      open(clidata, sendbuf, result);
      break;
    case 0x03: // close
      close(clidata, sendbuf, result);
      break;
    case 0x04: // seek
      DBGOUT_CPU("RpcCmd %02x is not supported.\n", p_fno);
      break;
    case 0x05: // read
      read(clidata, sendbuf, result);
      break;
    case 0x06: // write
      write(clidata, sendbuf, result);
      break;
    case 0x0a: // flush
      flush(clidata, sendbuf, result);
      break;
    case 0x0c: // chdir
      chdir(clidata, sendbuf, result);
      break;
    case 0x0d: // get dir
      getdir(clidata, sendbuf, result);
      break;
    case 0x0e: // set info
      DBGOUT_CPU("RpcCmd %02x is not supported.\n", p_fno);
      break;
    case 0x0f: // delete
      del(clidata, sendbuf, result);
      break;
    case 0x10: // format
      format(clidata, sendbuf, result);
      break;
    case 0x11: // unformat
      DBGOUT_CPU("RpcCmd %02x is not supported.\n", p_fno);
      break;
    case 0x12: // get ent space
      DBGOUT_CPU("RpcCmd %02x is not supported.\n", p_fno);
      break;
    case 0x14: // change priority
      DBGOUT_CPU("RpcCmd %02x is not supported.\n", p_fno);
      break;
    case 0xfe: // init
      init(clidata, sendbuf, result);
      break;
    default:   // invalid
      DBGOUT_CPU("SifCallRpc MemCd Invalid FunctionID!!\n");
      break;
  }
}

void MemCard::getinfo(u32* p_client_data, u32* p_sendbuf, s32* p_result) {
  u32 port = p_sendbuf[1];
  u32 slot = p_sendbuf[2];
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

  u32 type;
  u32 free_size;
  if (exist && formatted) {
    // フォーマット済み
    p_result[0] = -1;
    type        = 2;
    free_size   = getDiskFree();
  } else if (exist && formatted == false) {
    // 未フォーマット
    p_result[0] = -2;
    type        = 2;
    free_size   = 0;
  } else {
    // 未接続
    p_result[0] = -10;
    type        = 0;
    free_size   = 0;
  }

  u32* work_buf = (u32*)VA2PA(p_sendbuf[7]);

  if (p_sendbuf[3] == 1) work_buf[0]  = type;
  if (p_sendbuf[4] == 1) work_buf[1]  = free_size;
  if (p_sendbuf[5] == 1) work_buf[36] = formatted;

  // 前回と同じなら result = 0 とする
  // これはMcSyncにて取得される
  if (get_info_last_state_[port].valid  == true &&
      get_info_last_state_[port].type   == type &&
      get_info_last_state_[port].free   == free_size &&
      get_info_last_state_[port].format == formatted) {
    p_result[0] = 0;
  } else {
    // 現在の状態を保存する
    get_info_last_state_[port].valid  = true;
    get_info_last_state_[port].type   = type;
    get_info_last_state_[port].free   = free_size;
    get_info_last_state_[port].format = formatted;
  }

  g_cpu->m_v0.d[0] = 0x00000000;
  DBGOUT_CPU("%s succeeded! port=%d slot=%d mcd_path=%s result=%d\n",
    __FUNCTION__, port, slot, mcd_path, p_result[0]);
}

void MemCard::open(u32* p_client_data, u32* p_sendbuf, s32* p_result) {
  enum {
    MODE_READ  = 0x0001,
    MODE_WRITE = 0x0002,
    MODE_RDWR  = 0x0003,
    MODE_NEW   = 0x0200,
    MODE_MKDIR = 0x0040,
  };
  u32 port  = p_sendbuf[0];
  u32 slot  = p_sendbuf[1];
  u32 mode  = p_sendbuf[2];
  s8* fname = (s8*)&p_sendbuf[5];

  s8 path[1024];
  if (strchr(fname, '/')) {
    // フルパスと見なす
    sprintf(path, "%s/memcard%d/%s",
      g_base_dir, slot + 1, fname);
  } else {
    sprintf(path, "%s/memcard%d/%s/%s",
      g_base_dir, slot + 1, relative_dir_, fname);
  }
  switch (mode & 0xff) {
    case MODE_MKDIR:
      if (!CreateDirectory((LPCSTR)path, NULL)) {
        p_result[0] = -4;
        break;
      }
      DBGOUT_CPU("%s succeeded! port=%d slot=%d mode=%04x fname=%s\n",
        __FUNCTION__, port, slot, mode, fname);
      p_result[0] = 0;
      break;
    case MODE_READ:
    case MODE_WRITE:
    case MODE_RDWR: {
      s8 modestr[16];
      if ((mode & 0xff) == MODE_READ) {
        strcpy(modestr, "rb");
      } else if ((mode & 0xff) == MODE_WRITE) {
        strcpy(modestr, "wb");
      } else if ((mode & 0xff) == MODE_RDWR) {
        strcpy(modestr, (mode & MODE_NEW) ? "w+b" : "r+b");
      }
      FILE* fp = fopen((LPCSTR)path, modestr);
      if (fp == NULL) {
        p_result[0] = -4;
        DBGOUT_CPU("%s failed! port=%d slot=%d mode=%04x fname=%s "
          "result=%d\n", __FUNCTION__, port, slot, mode, fname, p_result[0]);
        break;
      }
      
      McFileMgmt* file_mgmt = new McFileMgmt;
      file_mgmt->fp = fp;
      // ファイルデスクリプタ採番
      int fd = getUniqueFd();
      if (fd == -1) {
        p_result[0] = -5;
        DBGOUT_CPU("%s failed! port=%d slot=%d mode=%04x fname=%s "
          "result=%d\n", __FUNCTION__, port, slot, mode, fname, p_result[0]);
      } else {
        file_mgmt->fd = fd;
        file_mgmt_.push_back(file_mgmt);
        p_result[0] = fd;
        DBGOUT_CPU("%s succeeded! port=%d slot=%d mode=%04x "
          "fname=%s fd=%d\n", __FUNCTION__, port, slot, mode, fname, fd);
      }
    } break;
    default: p_result[0] = -4; break;
  }
  g_cpu->m_v0.d[0] = 0x00000000;
}

void MemCard::close(u32* p_client_data, u32* p_sendbuf, s32* p_result) {
  u32 fd = p_sendbuf[0];
  closeMcdFile(fd);

  p_result[0] = 0;
  g_cpu->m_v0.d[0] = 0x00000000;
  DBGOUT_CPU("%s succeeded! fd=%d\n", __FUNCTION__, fd);
}

void MemCard::read(u32* p_client_data, u32* p_sendbuf, s32* p_result) {
  u32 fd     = p_sendbuf[0];
  u32 buf_va = p_sendbuf[6];
  u32 size   = p_sendbuf[3];

  u8*   buf = (u8*)VA2PA(buf_va);
  FILE* fp  = getFilePointer(fd);

  u32* work_buf = (u32*)VA2PA(p_sendbuf[7]);
  work_buf[0] = 0;
  work_buf[1] = 0;
  work_buf[2] = 0;
  work_buf[3] = 0;

  // ファイルポインタが正しく認識されないため態々fseekしている。
  // SLPM_663.33でOPTIONメニューからのセーブ時にw+bで
  // 読み書きオープンしwrite後になぜかそのままreadを行っているが、
  // このreadによってwriteした位置から読み込みが行われ
  // 不正なメモリの読み込みが行われる（これによりバッファを破壊する）。
  // 不正に読み込まれた分だけ実際にディスク上に作成される
  // ファイルサイズも増えている。
  // ちなみにfgetposでファイルの位置を確認すると末尾を確かに指している。
  // fseekで現在位置を指すことで回避できるようなのでとりあえずよしとする。
  fseek(fp, 0, SEEK_CUR);

  size_t actual_size = fread(buf, 1, size, fp);
  p_result[0] = actual_size;
  g_cpu->m_v0.d[0] = 0x00000000;
  DBGOUT_CPU("%s succeeded! fd=%d buf=0x%08x size=%d\n",
    __FUNCTION__, fd, buf_va, size);
}

void MemCard::write(u32* p_client_data, u32* p_sendbuf, s32* p_result) {
  u32 fd     = p_sendbuf[0];
  u32 buf_va = p_sendbuf[6];
  u32 size   = p_sendbuf[3];

  u8* buf = (u8*)VA2PA(buf_va);
  FILE* fp = getFilePointer(fd);
  fseek(fp, 0, SEEK_CUR); // readと同様の問題を懸念。readのコメント参照
  size_t actual_size = fwrite(buf, 1, size, fp);
  p_result[0] = actual_size;
  g_cpu->m_v0.d[0] = 0x00000000;
  DBGOUT_CPU("%s succeeded! fd=%d buf=0x%08x size=%d\n",
    __FUNCTION__, fd, buf_va, size);
}

void MemCard::flush(u32* p_client_data, u32* p_sendbuf, s32* p_result) {
  u32 fd = p_sendbuf[0];

  FILE* fp = getFilePointer(fd);
  fflush(fp);

  p_result[0] = 0;
  g_cpu->m_v0.d[0] = 0x00000000;
  DBGOUT_CPU("%s succeeded! fd=%d\n", __FUNCTION__, fd);
}

void MemCard::chdir(u32* p_client_data, u32* p_sendbuf, s32* p_result) {
  u32 port = p_sendbuf[0];
  u32 slot = p_sendbuf[1];
  s8* path = (s8*)&p_sendbuf[5];
  u32 pwd  = p_sendbuf[4];

  if (pwd) {
    s8* buf = (s8*)VA2PA(pwd);
    strcpy(buf, relative_dir_);
  }

  strcpy(relative_dir_, path);

  p_result[0] = 0;
  g_cpu->m_v0.d[0] = 0x00000000;
  DBGOUT_CPU("%s succeeded! port=%d slot=%d path=%s\n",
    __FUNCTION__, port, slot, path);
}

void MemCard::getdir(u32* p_client_data, u32* p_sendbuf, s32* p_result) {
  u32 port   = p_sendbuf[0];
  u32 slot   = p_sendbuf[1];
  u32 next   = p_sendbuf[2];
  u32 maxent = p_sendbuf[3];
  u32 table  = p_sendbuf[4];
  u8* name   = (u8*)&p_sendbuf[5];

  s8 file_path[1024];
  sprintf(file_path, "%s/memcard%d/%s", g_base_dir, slot + 1, name);

  if (next == 0) {
    // 最初から取り直し
    get_dir_count_ = 0;
  }
  McGetDirEntry* ent_table = (McGetDirEntry*)VA2PA(table);
  WIN32_FIND_DATA find_data;
  HANDLE hdl = FindFirstFile(file_path, &find_data);
  if (hdl == INVALID_HANDLE_VALUE) {
    p_result[0] = -4;
  } else {
    BOOL succeeded;
    u32 file_cnt = 0;
    u32 ent_cnt = 0;
    do {
      if (file_cnt >= get_dir_count_) {
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
        get_dir_count_++;
      }
      file_cnt++;
      succeeded = FindNextFile(hdl, &find_data);
    } while (succeeded && ent_cnt < maxent);
    FindClose(hdl);
    p_result[0] = ent_cnt;
  }
  g_cpu->m_v0.d[0] = 0x00000000;
  DBGOUT_CPU("%s succeeded! port=%d slot=%d next=%d maxent=%d name=%s "
    "result=%d\n", __FUNCTION__, port, slot, next, maxent, name, p_result[0]);
}

void MemCard::del(u32* p_client_data, u32* p_sendbuf, s32* p_result) {
  u32 port  = p_sendbuf[0];
  u32 slot  = p_sendbuf[1];
  s8* fname = (s8*)&p_sendbuf[5];

  s8 path[1024];
  if (strchr(fname, '/')) {
    // フルパスと見なす
    sprintf(path, "%s/memcard%d/%s",
      g_base_dir, slot + 1, fname);
  } else {
    sprintf(path, "%s/memcard%d/%s/%s",
      g_base_dir, slot + 1, relative_dir_, fname);
  }

  DeleteFile(path);

  p_result[0] = 0;
  g_cpu->m_v0.d[0] = 0x00000000;
  DBGOUT_CPU("%s succeeded! port=%d port=%d path=%s\n",
    __FUNCTION__, port, slot, path);
}

void MemCard::format(u32* p_client_data, u32* p_sendbuf, s32* p_result) {
  u32 port = p_sendbuf[0];
  u32 slot = p_sendbuf[1];

  s8 mcd_path[1024];
  sprintf(mcd_path, "%s/memcard%d", g_base_dir, p_sendbuf[1] + 1);
  if (CreateDirectory(mcd_path, NULL)) {
    p_result[0] = 0;

    get_info_last_state_[port].valid  = true;
    get_info_last_state_[port].type   = 2;
    get_info_last_state_[port].free   = getDiskFree();
    get_info_last_state_[port].format = 1;
  } else {
    p_result[0] = -1;
  }
  g_cpu->m_v0.d[0] = 0x00000000;
  DBGOUT_CPU("%s succeeded! port=%d "
    "slot=%d result=%d\n", __FUNCTION__, port, slot, p_result[0]);
}

void MemCard::init(u32* p_client_data, u32* p_sendbuf, s32* p_result) {
  // Version取得
  relative_dir_[0] = '\0';
  get_dir_count_ = 0;
  p_result[0] = 0;
  p_result[1] = 0x00000210;
  p_result[2] = 0x00000222;
  DBGOUT_CPU("%s succeeded!\n", __FUNCTION__);
}

u32 MemCard::getDiskFree() {
  ULARGE_INTEGER use, total, free;
  GetDiskFreeSpaceEx(g_base_dir, &use, &total, &free);
  return free.HighPart > 0 ? 0x7fffffff : free.LowPart;
}

FILE* MemCard::getFilePointer(int p_fd) {
  for (u32 i = 0; i < file_mgmt_.size(); i++) {
    if (file_mgmt_[i]->fd == p_fd) {
      return file_mgmt_[i]->fp;
    }
  }
  return NULL;
}

void MemCard::closeMcdFile(int p_fd) {
  for (u32 i = 0; i < file_mgmt_.size(); i++) {
    if (file_mgmt_[i]->fd == p_fd) {
      fclose(file_mgmt_[i]->fp);
      delete file_mgmt_[i];
      file_mgmt_.erase(&file_mgmt_[i]);
      return;
    }
  }
}

int MemCard::getUniqueFd() {
  for (int i = 0; i < 256; i++) {
    bool unique = true;
    for (u32 j = 0; j < file_mgmt_.size(); j++) {
      if (file_mgmt_[j]->fd == i) {
        unique = false;
        break;
      }
    }
    if (unique) return i;
  }
  return -1;
}
