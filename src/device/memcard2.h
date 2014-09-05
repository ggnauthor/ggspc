#ifndef _ggsext_memcard2
#define _ggsext_memcard2

#include "main.h"
#include "stlp_wrapper.h"

struct Mc2SocketParam {
  u32 option;
  s32 port;
  s32 unknown1;
  s32 slot;
  s32 unknown2;
  s8  name[16];
};

struct Mc2SocketInfo {
  s32 port;
  s32 slot;
};

struct Mc2GetInfoState {
  int type;
  int free;
  int format;
};

struct Mc2GetDirEntry {
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
  u16 dummy;
  s8  name[32];
};

class MemCard2 {
public:
  MemCard2();
  ~MemCard2();

  void clearSocket();

  void callMemCard2Rpc(u32 p_client_data_ptr, u32 p_fno,
    u32 p_send_buf_ptr, u32 p_send_buf_size,
    u32 p_recv_buf_ptr, u32 p_recv_buf_size);
  void getinfo(u32 p_socket, u32 p_buf_va);
  void read(u32 p_socket, u32 p_name_va,
    u32 p_buf_va, u32 p_ofs, u32 p_size);
  void write(u32 p_socket, u32 p_name_va,
    u32 p_buf_va, u32 p_ofs, u32 p_size);
  void mkdir(u32 p_socket, u32 p_name_va);
  void getdir(u32 p_socket, u32 p_name_va,
    u32 p_ofs, u32 p_maxent, u32 p_buf_va, u32 p_cnt_va);
  void chdir(u32 p_socket, u32 p_name_va, u32 p_pwd_va);
  void format(u32 p_socket);

private:
  u32   getDiskFree();

  // 最後にMcGetInfoを呼び出したときの状態を保存する
  Mc2GetInfoState get_info_last_state_[2];
  // 取得済みソケット
  vector<Mc2SocketInfo*> socket_;
  // 相対パス
  s8  relative_dir_[1024];
};

#endif // _ggsext_memcard2
