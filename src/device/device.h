#ifndef _ggsext_ggsext
#define _ggsext_ggsext

#include "main.h"
#include "cpu/r5900.h"

enum eProduct {
  kProduct_GGX,
  kProduct_GGXX,
  kProduct_GGXXR,
  kProduct_GGISUKA,
  kProduct_GGXXS,
  kProduct_GGXXACP,
  kProduct_Invalid = -1,
};

struct FileMgmtTab {
  s8  path[96];
  u32 buffer;
  u32 file_offset;
  u32 file_size;
  int _a;
  int _b;
  int _c;
  int _d;
  int id;
  int _e;
  int _f;
};

struct ImageRefData {
  u32 ref_id;
  u32 _04;
  u32 _08;
  u32 tex_w_bits;
  u32 tex_h_bits;
  u32 _14;
  u32 _18;
  u8* encoded_data;
  u16 enc_type;
  u16 tex_w;
  u16 tex_h;
  u16 request;
  u8* decoded_data;
  u32 render_mode;
  u32 idx;
  u16 bpp;
  u16 fixed_pal;
  u32 _38;
  u32 img_id;
};

struct BattleObjectInfo {
  u16 chara_id;
  u8  dir;
  u8  dir_internal;
  u32 flag;
  u32 _08;
  u32 _0c;
  u16 action_id;
  u16 _12;
  u16 _14;
  u16 life;
  u32 parent_info_ptr;
  u16 _1c;
  u8  _1e;
  u8  player_side;
  u32 _20[4];
  u32 _30[4];
  u32 _40[4];
  u32 _50[4];
  u32 _60[4];
  u32 _70[4];
  u32 _80;
  u32 _84;
  u32 pal_method;
  u8  pal_id;
  u8  extr_pal_ofs;
  u8  use_extr_pal;
  u8  _8f;
  u16 _90;
  u16 _92;
  u16 _94;
  u16 image_idx;
  u32 _98[2];
  u32 _a0;
  u32 pos_x;
  u32 pos_y;
  u32 _a3;
  u32 _b0[4];
  u32 _c0[4];
  u16 angle;
  u16 prio;
  u32 _d4;
  u32 _d8;
  u32 _dc;
  u32 _e0[4];
  u32 _f0[4];
  u32 _100;
};

struct BattleObjectInfo669_65 {
  u16 chara_id;
  u8  dir;
  u8  dir_internal;
  u32 _04;
  u32 _08;
  u32 flag;
  u32 _10;
  u32 _14;
  u16 action_id;
  u16 _1a;
  u16 _1c;
  u16 life;
  u32 parent_info_ptr;
  u16 _24;
  u8  _26;
  u8  player_side;
  u32 _28[4];
  u32 _38[4];
  u32 _48[4];
  u32 _58[4];
  u32 _68[4];
  u32 _78[4];
  u32 _88;
  u32 _8c;
  u32 pal_method;
  u8  pal_id;
  u8  extr_pal_ofs;
  u8  use_extr_pal;
  u8  _97;
  u16 _98;
  u16 _9a;
  u16 _9c;
  u16 image_idx;
  u32 _a0[2];
  u32 _a8;
  u32 pos_x;
  u32 pos_y;
  u32 _a3;
  u32 _b8[4];
  u32 _c8[4];
  u16 angle;
  u16 prio;
  u32 _dc;
  u32 _e0;
  u32 _e4;
  u32 _e8[4];
  u32 _f8[4];
  u32 _108;
  u32 _10c;
  u32 _110;
  u32 _114;
  u32 _118;
  u32 _11c;
  u32 _120;
};

extern class SettingFileMgr* g_setting_file_mgr;

class GraphicMgr;
class SoundDriver;
class InputMgr;
class Mpeg2Decoder;
class MemCard;
class MemCard2;

class Device {
  friend DWORD CALLBACK vbl_proc(LPVOID lpParameter);
public:
  Device();
  ~Device();

  bool init();
  void clean();

  void handleSyscall(int p_code);
  void callEEFuncFromExecFiber(u32 p_func, u32 p_args);
  u32* getCharaPaletteAddr(u32 p_chara_mode, u32 p_chara_id);

  void activate();
  bool isRenderingNow();
#if !defined _DEBUGGER
  void toggleScreenMode();
  void changeWindowMode();
  void restoreDevice();
#endif // !defined _DEBUGGER

  s8* product_name() { return product_name_; }

  GraphicMgr*      graph_mgr() { return graph_mgr_; }
  SoundDriver*     sound_mgr() { return sound_mgr_; }
  InputMgr*        input_mgr() { return input_mgr_; }
  Mpeg2Decoder*    mpeg2dec() { return mpeg2dec_; }
  
  ImageRefData* ps2_img_ref_data(int p_idx) {
    return &ps2_img_ref_data_[p_idx];
  }
  u32 ps2_max_img_ref_count() { return ps2_max_img_ref_count_; }
  u32 ps2_render_mode() { return *ps2_render_mode_; }
  u32 ps2_clear_color() { return *ps2_clear_color_; }
  u32 ps2_chara_boi_va() { return *ps2_chara_boi_va_; }
  u32 ps2_drawing_boi_va() { return *ps2_drawing_boi_va_; }
  u16 ps2_chara_pal_id(int p_idx) {
    if (ps2_chara_pal_id_) {
      return ps2_chara_pal_id_[p_idx];
    } else {
      return ps2_chara_pal_id_1byte_[p_idx];
    }
  }

  u32* ps2_dynamic_pal_addr(int p_idx) {
    return ps2_dynamic_pal_addr_[p_idx];
  }

  u32 ps2_character_mode(int p_idx) {
    return *ps2_character_mode_[p_idx];
  }

  eProduct getProduct(s8* p_product_cd);

private:
  bool initFileLoadThread();
  void cleanFileLoadThread();
  void initInputMgr();
  void cleanInputMgr();
  void                bindPs2Address(s8* p_product_cd);
  static DWORD WINAPI fileLoadThreadProc(LPVOID p_param);

  bool initialized_;
  s8   product_name_[64];

  volatile bool file_load_thread_end_;
  HANDLE        file_load_thread_hdl_;

  GraphicMgr*   graph_mgr_;
  SoundDriver*  sound_mgr_;
  InputMgr*     input_mgr_;
  Mpeg2Decoder* mpeg2dec_;
  MemCard*      memcard_;
  MemCard2*     memcard2_;

  u32           vbl_intc_tid_;    // VBL割り込み用スレッドID
  bool          vbl_intc_enable_; // VBL割り込み有効フラグ
  u32           vbl_time0_;
  f64           vbl_diff_;
  volatile u32  vbl_count_;

  // PS2のメモリにアクセスする為のポインタ
  ImageRefData* ps2_img_ref_data_;         // 画像参照データ
  u32           ps2_max_img_ref_count_;    // 画像参照データ数
  u32*          ps2_render_mode_;          // アルファブレンディングモード
  u32*          ps2_clear_color_;          // クリアカラー
  u32*          ps2_chara_boi_va_;         // キャラのバトルオブジェクト情報
  u32*          ps2_drawing_boi_va_;       // 描画中のバトルオブジェクト情報
  u16*          ps2_chara_pal_id_;         // パレットID
  u8*           ps2_chara_pal_id_1byte_;   // パレットID(1byte)
  u32*          ps2_dynamic_pal_addr_[2];  // 動的パレット[1P,2P]
  u32*          ps2_pal_addr_table_[3];    // 全キャラ全パレットアドレス
                                           // [Normal,Ex,#Reload]
  u32*          ps2_character_mode_[2];    // キャラモード[1P,2P]
                                           // 0=Normal 1=Ex 2=#Reload
  FileMgmtTab*  ps2_file_target_;          // ファイル読込対象
  volatile u32* ps2_file_busy_;            // ファイル利用中フラグ

  u32*          ps2_movie_dst_buf_ptr_;    // ムービー展開済バッファポインタ
  u32*          ps2_movie_dst_buf_size_;   // ムービー展開済バッファサイズ
  u32*          ps2_movie_src_buf_ptr_;    // ムービーRAWデータポインタ
  u32*          ps2_movie_src_buf_size_;   // ムービーRAWデータポインタ
  u32*          ps2_movie_buf_unproc_size_;// ムービーバッファ未処理サイズ
  u32*          ps2_movie_buf_proc_size_;  // ムービーバッファ処理済サイズ
  u32*          ps2_movie_buf_proc_pos_;   // ムービーバッファ処理済位置

  static LARGE_INTEGER freq_;
  static BOOL          support_performance_counter_;
};

DWORD CALLBACK vbl_proc(LPVOID lpParameter);

extern Device* g_dev_emu;

#endif // _ggsext_ggsext
