#ifndef _ggsext_memcard
#define _ggsext_memcard

#include "main.h"
#include "stlp_wrapper.h"

struct McGetInfoState {
  bool valid;
  int  type;
  int  free;
  int  format;
};

struct McFileMgmt {
  int   fd;
  FILE* fp;
};

struct McGetDirEntry {
  struct {
    u8  dummy;
    u8  sec;
    u8  min;
    u8  hour;
    u8  day;
    u8  month;
    u16 year;
  } create, modify;
  u32 size;
  u16 attr;
  u16 dummy[5];
  s8  name[32];
};

class MemCard {
public:
  MemCard();
  ~MemCard();

  void callMemCardRpc(u32 p_client_data_ptr, u32 p_fno,
    u32 p_send_buf_ptr, u32 p_send_buf_size,
    u32 p_recv_buf_ptr, u32 p_recv_buf_size);

private:
  void getinfo(u32* p_client_data, u32* p_sendbuf, s32* p_result);
  void open(u32* p_client_data, u32* p_sendbuf, s32* p_result);
  void close(u32* p_client_data, u32* p_sendbuf, s32* p_result);
  void read(u32* p_client_data, u32* p_sendbuf, s32* p_result);
  void write(u32* p_client_data, u32* p_sendbuf, s32* p_result);
  void flush(u32* p_client_data, u32* p_sendbuf, s32* p_result);
  void chdir(u32* p_client_data, u32* p_sendbuf, s32* p_result);
  void getdir(u32* p_client_data, u32* p_sendbuf, s32* p_result);
  void del(u32* p_client_data, u32* p_sendbuf, s32* p_result);
  void format(u32* p_client_data, u32* p_sendbuf, s32* p_result);
  void init(u32* p_client_data, u32* p_sendbuf, s32* p_result);

  u32   getDiskFree();
  FILE* getFilePointer(int p_fd);
  void  closeMcdFile(int p_fd);
  int   getUniqueFd();

  // 最後にMcGetInfoを呼び出したときの状態を保存する
  McGetInfoState get_info_last_state_[2];
  // PS2のファイルデスクリプタとWindowsのファイルポインタとのマッピング
  vector<McFileMgmt*> file_mgmt_; 
  // McGetDirで取得済みファイル／ディレクトリ数
  u32 get_dir_count_;
  // 相対パス
  s8  relative_dir_[1024];
};

#endif // _ggsext_memcard
