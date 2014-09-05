#include "main.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "thread/thread.h"
#include "thread/semaphore.h"
#if defined _DEBUGGER
#include "debugger/debugger.h"
#endif
#include "device/device.h"
#include "device/graphic.h"
#include "device/audio.h"
#include "device/input.h"
#include "device/memcard.h"
#include "device/memcard2.h"
#include "device/mpeg2dec.h"
#include "device/setting.h"
#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

LARGE_INTEGER Device::freq_;
BOOL          Device::support_performance_counter_;

extern PS2ThreadMgr*    thread_mgr_;
extern PS2SemaphoreMgr* sema_mgr_;

Device* g_dev_emu = NULL;

Device::Device() :
    initialized_(false),
    file_load_thread_end_(false),
    file_load_thread_hdl_(NULL),
    graph_mgr_(NULL),
    sound_mgr_(NULL),
    input_mgr_(NULL),
    mpeg2dec_(NULL),
    memcard_(NULL),
    vbl_intc_tid_(0x00000000),
    vbl_intc_enable_(false),
    vbl_diff_(0.0),
    vbl_count_(0),
    ps2_img_ref_data_(NULL),
    ps2_render_mode_(NULL),
    ps2_clear_color_(NULL),
    ps2_drawing_boi_va_(NULL),
    ps2_chara_boi_va_(NULL),
    ps2_chara_pal_id_(NULL),
    ps2_chara_pal_id_1byte_(NULL),
    ps2_file_target_(NULL),
    ps2_file_busy_(NULL),
    ps2_movie_dst_buf_ptr_(NULL),
    ps2_movie_dst_buf_size_(NULL),
    ps2_movie_src_buf_ptr_(NULL),
    ps2_movie_src_buf_size_(NULL),
    ps2_movie_buf_unproc_size_(NULL),
    ps2_movie_buf_proc_size_(NULL),
    ps2_movie_buf_proc_pos_(NULL) {
  product_name_[0] = '\0';
  ps2_dynamic_pal_addr_[0] = NULL;
  ps2_dynamic_pal_addr_[1] = NULL;
  ps2_pal_addr_table_[0] = NULL;
  ps2_pal_addr_table_[1] = NULL;
  ps2_pal_addr_table_[2] = NULL;
  ps2_character_mode_[0] = NULL;
  ps2_character_mode_[1] = NULL;
  vbl_time0_ = timeGetTime();
}

Device::~Device() {
}

bool Device::init() {
  if (initialized_ == false) {
    initInputMgr();

    graph_mgr_ = new GraphicMgr();
    if (graph_mgr_->init() == false) {
      return false;
    }
    graph_mgr_->setup();

    sound_mgr_ = new SoundDriver();
    if (sound_mgr_->init() == false) {
      return false;
    }

    mpeg2dec_ = new Mpeg2Decoder();

    memcard_ = new MemCard();
    memcard2_ = new MemCard2();

    bindPs2Address(g_elf_name);

    support_performance_counter_ = QueryPerformanceFrequency(&freq_);

    initFileLoadThread();
    initialized_ = true;
  }
  return true;
}

void Device::clean() {
  if (initialized_) {
    initialized_ = false;
    cleanFileLoadThread();

    if (mpeg2dec_) {
      delete mpeg2dec_;
      mpeg2dec_ = NULL;
    }
    
    if (sound_mgr_) {
      sound_mgr_->clean();
      delete sound_mgr_;
      sound_mgr_ = NULL;
    }

    if (graph_mgr_) {
      graph_mgr_->clean();
      delete graph_mgr_;
      graph_mgr_ = NULL;
    }

    cleanInputMgr();
    
    delete memcard2_;
    delete memcard_;
  }
}

void debugout_ggs(bool p_addlf) {
  u8 arg_reg_tab[] = {
  // a1 a2 a3 t0 t1  t2  t3 v0 v1
      5, 6, 7, 8, 9, 10, 11, 2, 3,
  };
  u32 args[16];
  int argidx = 0;
  s8* str = (s8*)VA2PA(g_cpu->m_a0.d[0]);
  bool per = false;
  while (1) {
    if (per) {
      switch (*str) {
        case 's':
          args[argidx++] = VA2PA(g_cpu->m_gpr[arg_reg_tab[argidx]].d[0]);
          per = false;
          break;
        case 'd': case 'o': case 'x': case 'f': case 'e':
          args[argidx++] = g_cpu->m_gpr[arg_reg_tab[argidx]].d[0];
          per = false;
          break;
        case 'c':
          args[argidx++] = g_cpu->m_gpr[arg_reg_tab[argidx]].d[0] & 0xff;
          per = false;
          break;
        case '-': case '#': case '.': case '0': case '1':
        case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
          break;
        default:
          per = false;
          break;
      }
    } else {
      str = strchr(str, '%');
      if (str == NULL) break;

      if (*(str + 1) == '%') {
        str++;
        per = false;
      } else {
        per = true;
      }
    }
    str++;
  }
  DBGOUT_GGS((s8*)VA2PA(g_cpu->m_a0.d[0]),
             args[0], args[1], args[2], args[3],
             args[4], args[5], args[6], args[7], args[8]);
  if (p_addlf) {
    DBGOUT_GGS("\n");
  }
}

void Device::handleSyscall(int p_code) {
  switch (p_code) {
    case 0x0010: { // AddIntcHandler
      if (g_cpu->m_a0.d[0] == 2) { // VBLなら
        u32 new_pc = 0x00080050; // ggsext::intc_loop
        u32 new_sp = 0x00100000;
        u32 new_gp = 0x003b2af0;
        // スタックの末尾8バイトには割り込み処理アドレスを設定
        u32* intc_proc = (u32*)VA2PA(new_sp - 4);
        *intc_proc = g_cpu->m_a1.d[0];
        // スタックの末尾4バイトには判定用syscallコードを設定
        u32* cond_sccode = (u32*)VA2PA(new_sp - 8);
        *cond_sccode = 0x00002024; // VBL割り込み実行判定のSyscallコード
        vbl_intc_tid_ = thread_mgr_->create(new_pc,
                                            new_sp - 8,
                                            new_gp);
        DBGOUT_CPU("CreateThread(VBL Interrupt) id=%d, entry=%08x\n",
                   vbl_intc_tid_, new_pc);
      }
    } break;
    case 0x0011: { // RemoveIntcHandler
      thread_mgr_->remove(vbl_intc_tid_);
    } break;
    case 0x0014: { // _EnableIntc
      vbl_intc_enable_ = true;
    } break;
    case 0x0015: { // _DisableIntc
      vbl_intc_enable_ = false;
    } break;
    case 0x0020: { // CreateThread
      PS2ThreadParam* param = (PS2ThreadParam*)VA2PA(g_cpu->m_a0.d[0]);
      g_cpu->m_v0.q[0] = thread_mgr_->create(
        (u32)param->entry_addr,
        (u32)param->stack_addr + param->stack_size,
        (u32)param->global_ptr_reg);
      DBGOUT_CPU("CreateThread id=%d, entry=%08x, "
                 "stack=%08x(%04x), gp=%08x\n",
                 g_cpu->m_v0.q[0], param->entry_addr, param->stack_addr,
                 param->stack_size, param->global_ptr_reg);
    } break;
    case 0x0021: // DeleteThread
      thread_mgr_->remove(g_cpu->m_a0.d[0]);
      break;
    case 0x0022: // StartThread
      if (g_cpu->m_a0.d[0] < thread_mgr_->thread_count()) {
        DBGOUT_CPU("StartThread  id=%d, args=%08x\n",
                   g_cpu->m_a0.d[0], g_cpu->m_a1.d[0]);
        thread_mgr_->switchTo(g_cpu->m_a0.d[0]);
      } else {
        DBGOUT_CPU("StartThread Failed. Invalid Thread id=%d\n",
                   g_cpu->m_a0.d[0]);
      }
      break;
    case 0x002f: // GetThreadId
      g_cpu->m_v0.q[0] = thread_mgr_->cur_thread_id();
      break;
    case 0x003e: // EndOfHeap
      g_cpu->m_v0.q[0] = 0x02000000;
      break;
    case 0x003c: // RFU060
      // スタックポインタを返す
      g_cpu->m_v0.q[0] = 0x02000000 - 0x12a0;
      break;
    case 0x0040: { // CreateSema
      PS2SemaParam* param = (PS2SemaParam*)VA2PA(g_cpu->m_a0.d[0]);
      g_cpu->m_v0.q[0] =
        sema_mgr_->create(param->init_count, param->max_count);
      DBGOUT_CPU("CreateSema id=%d, pc=%08x, count=%d/%d\n",
                 g_cpu->m_v0.q[0], g_cpu->m_ra.d[0] - 8,
                 param->init_count, param->max_count);
    } break;
    case 0x0041: // DeleteSema
      sema_mgr_->remove(g_cpu->m_a0.d[0]);
      break;
    case 0x0042: // SignalSema
    case 0xffbd: // iSignalSema
      if (sema_mgr_->signal(g_cpu->m_a0.d[0])) {
        //DBGOUT_CPU("SignalSema id=%d count=%d/%d\n",
        //      g_cpu->m_a0.d[0],
        //      g_sema_array[g_cpu->m_a0.d[0]]->get_count(),
        //      g_sema_array[g_cpu->m_a0.d[0]]->get_max());
        thread_mgr_->switchTo();
        g_cpu->m_v0.d[0] = g_cpu->m_a0.d[0];
      } else {
        g_cpu->m_v0.d[0] = -1;
      }
      break;
    case 0x0044:   // WaitSema
    case 0x0045:   // PollSema
    case 0xffba: { // iPollSema
      int sema_id = g_cpu->m_a0.d[0];
      int res = sema_mgr_->wait(sema_id);
      if (res == 1) {
          g_cpu->m_v0.d[0] = sema_id;
      } else if (res == 2) {
        if (p_code == 0x0044) { // WaitSema
          // syscallは必ずretするのでPC戻すだけで停止させられる
          g_cpu->m_pc -= 4;
          thread_mgr_->switchTo();
          g_cpu->m_v0.d[0] = sema_id;
        } else {
          g_cpu->m_v0.d[0] = -1;
        }
        //DBGOUT_CPU("WaitSema   id=%d count=%d/%d\n",
        //  sema_id,
        //  g_sema_array[sema_id]->get_count(),
        //  g_sema_array[sema_id]->get_max(),
        //  g_cur_thread);
        //DBGOUT_CPU("------------------------------------\n");
      } else {
        g_cpu->m_v0.d[0] = -1;
      }
    } break;
    case 0x76:  // sceSifDmaStat
      g_cpu->m_v0.q[0] = -1;
      break;
    case 0x77: { // sceSifSetDma
      u32 src = *((u32*)VA2PA(g_cpu->m_a0.d[0]));
      u32 dst = *((u32*)VA2PA(g_cpu->m_a0.d[0] + 4));
      u32 size= *((u32*)VA2PA(g_cpu->m_a0.d[0] + 8));

      DBGOUT_CPU("sceSifSetDma dst=%08x, src=%08x, size=%08x\n",
                 dst, src, size);
      memcpy((void*)VA2PA(dst), (void*)VA2PA(src), size);
      g_cpu->m_v0.q[0] = 1;
    } break;
    case 0x2000:  // rpc call
      if (getProduct(g_elf_name) == kProduct_GGXX) {
        switch (g_cpu->m_a0.d[0]) {
          case 0x340170:
            sound_mgr_->call_snddrv_rpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0],(u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;
          case 0x38ce80:
            sound_mgr_->call_isdsrv_rpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], (u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;
          case 0x399200:
            memcard_->callMemCardRpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], (u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;
        }
      } else if (getProduct(g_elf_name) == kProduct_GGXXR) {
        switch (g_cpu->m_a0.d[0]) {
          case 0x369030:
            sound_mgr_->call_snddrv_rpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0],(u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;
          case 0x3b6000:
            sound_mgr_->call_isdsrv_rpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], (u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;
          case 0x3c5400:
            memcard_->callMemCardRpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], (u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;
        }
      } else if (getProduct(g_elf_name) == kProduct_GGXXS) {
        switch (g_cpu->m_a0.d[0]) {
          case 0x3d5cb0:
            sound_mgr_->call_snddrv_rpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0],(u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;
          case 0x3b3300:
            sound_mgr_->call_isdsrv_rpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], (u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;
          case 0x41b700:
            memcard_->callMemCardRpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], (u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;    
        }
      } else if (getProduct(g_elf_name) == kProduct_GGXXACP) {
        switch (g_cpu->m_a0.d[0]) {
          case 0x478370:
            sound_mgr_->call_snddrv_rpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], (u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;
          case 0x454600:
            sound_mgr_->call_isdsrv_rpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], (u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;
          case 0x4a2940:
            memcard2_->callMemCard2Rpc(
              g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], (u32)g_cpu->m_a3.d[0],
              g_cpu->m_t0.d[0], (u32)g_cpu->m_t1.d[0], g_cpu->m_t2.d[0]);
            break;
        }
      }
      // 終了時コールバックが指定されていればPS2上のコードを呼び出す
      if (g_cpu->m_t3.d[0]) {
        callEEFuncFromExecFiber(g_cpu->m_t3.d[0], g_cpu->m_s0.d[0]);
      }
      break;
    case 0x2001:  // draw primitive
      graph_mgr_->drawTriangleList(g_cpu->m_a0.d[0],
        (u8*)VA2PA(g_cpu->m_a1.d[0]), g_cpu->m_a2.d[0], g_cpu->m_a3.d[0]);
      break;
    case 0x2002:  // render start
      graph_mgr_->renderStart();
      break;
    case 0x2003:  // render end
      graph_mgr_->renderEnd();
  #if defined _DEBUGGER
      g_cpu->create_recompiler_hint();
  #endif // defined _DEBUGGER
      break;
    case 0x2004:  // decode all images
      graph_mgr_->decodeAllImages();
      break;
    case 0x2005:  // get input
      g_cpu->m_v0.d[0] = input_mgr_->getTargetState(g_cpu->m_s1.d[0]);
#if defined _DEBUG
      // F9押してる間、詳細情報を表示
      *((u32*)VA2PA(0x33a16c)) =
        (GetAsyncKeyState(VK_F9) & 0x8000) ? 0x0000feb7 : 0x00000000;
      *((u32*)VA2PA(0x2a2b44)) =
        (GetAsyncKeyState(VK_F9) & 0x8000) ? 0x00000001 : 0x00000000;
#endif // defined _DEBUG
      break;
    case 0x2006:  // draw primitive without texture mapping
      graph_mgr_->drawTriangleList(g_cpu->m_a0.d[0],
        (u8*)VA2PA(g_cpu->m_a1.d[0]), 0xffffffff, g_cpu->m_a2.d[0]);
      break;
    case 0x2007:  // update input
      input_mgr_->update();
      break;
    case 0x2008:  // debug output append LF
      debugout_ggs(false);
      break;
    case 0x2009:  // debug output
      debugout_ggs(true);
      break;
    case 0x2010:  // mc2 getinfo handler
      memcard2_->getinfo(g_cpu->m_a0.d[0], g_cpu->m_a1.d[0]);
      break;
    case 0x2011:  // mc2 getdir handler
      memcard2_->getdir(g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], g_cpu->m_a2.d[0],
                        g_cpu->m_a3.d[0], g_cpu->m_t0.d[0], g_cpu->m_t1.d[0]);
      break;
    case 0x2012:  // mc2 mkdir handler
      memcard2_->mkdir(g_cpu->m_a0.d[0], g_cpu->m_a1.d[0]);
      break;
    case 0x2013:  // mc2 write handler
      memcard2_->write(g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], g_cpu->m_a2.d[0],
                       g_cpu->m_a3.d[0], g_cpu->m_t0.d[0]);
      break;
    case 0x2014:  // mc2 read handler
      memcard2_->read(g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], g_cpu->m_a2.d[0],
                      g_cpu->m_a3.d[0], g_cpu->m_t0.d[0]);
      break;
    case 0x2015:  // mc2 chdir handler
      memcard2_->chdir(g_cpu->m_a0.d[0], g_cpu->m_a1.d[0], g_cpu->m_a2.d[0]);
      break;
    case 0x2016:  // mc2 format handler
      memcard2_->format(g_cpu->m_a0.d[0]);
      break;
    case 0x2020:  // draw movie picture
      graph_mgr_->drawMoviePicture(
        ps2_movie_dst_buf_ptr_,
        mpeg2dec_->comfirm_render());
      break;
    case 0x2021:  // movie decode
      mpeg2dec_->get_pic((u32*)VA2PA(*ps2_movie_dst_buf_ptr_));
      break;
    case 0x2022: { // movie load
      bool retry;
      do {
        retry = false;
        int read_blocks = *ps2_movie_buf_unproc_size_;
        if (*ps2_movie_buf_proc_pos_ + read_blocks > *ps2_movie_src_buf_size_) {
          read_blocks = *ps2_movie_src_buf_size_ - *ps2_movie_buf_proc_pos_;

          // ソースバッファの末尾では十分な
          // バッファリングが行えない為、再実行が必要
          retry = true;
        }

        int read_size =
          mpeg2dec_->fill_buffer(
            (u8*)VA2PA(*ps2_movie_src_buf_ptr_) + (*ps2_movie_buf_proc_pos_ << 11),
            read_blocks << 11
          ) >> 11;
        if (read_size == 0) break;

        *ps2_movie_buf_unproc_size_ -= read_size;
        *ps2_movie_buf_proc_size_   += read_size;

        *ps2_movie_buf_proc_pos_ =
          (*ps2_movie_buf_proc_pos_ + read_size) % *ps2_movie_src_buf_size_;
      } while (retry);
    } break;
    case 0x2023: // movie reset
      graph_mgr_->clearMovieTexture();
      mpeg2dec_->reset();
      memset((u32*)VA2PA(*ps2_movie_dst_buf_ptr_), 0, *ps2_movie_dst_buf_size_);
      break;
    case 0x2024: {// intc exec judge
      // VBL割り込みハンドラの実行判定を行う
      static u32 last_fps_count = -1;
      if (vbl_intc_enable_ && vbl_count_ != last_fps_count) {
        g_cpu->m_v0.d[0] = 1;
        last_fps_count = vbl_count_;
        break;
      }
      g_cpu->m_v0.d[0] = 0;
    } break;
    case 0x2025: // change next fiber
      thread_mgr_->switchTo();
      break;
    case 0x2026: // sleep
      Sleep(g_cpu->m_a0.d[0]);
      break;
  }
}

void Device::callEEFuncFromExecFiber(u32 p_func_va, u32 p_args) {
  // 実行ファイバ上のx86コードからEEコードを呼ぶ場合に使用
  // 具体的にはシステムコールのコールバックがそれにあたる
  u32 ret_addr;
  u32 save_pc = g_cpu->m_pc;
  u64 save_ra = g_cpu->m_ra.q[0];
  u64 save_v0 = g_cpu->m_v0.q[0];
  //u32 save_ebp;

  //R5900REGS save_regs = *(R5900REGS*)g_cpu;
  // raをこの関数の末尾に設定する
  // EEのアドレスと区別するために最上位ビットを立てる
  _asm {
    pushad;   // レジスタ退避
    mov eax, _ret;
    or eax, 0x80000000;
    mov dword ptr [ret_addr], eax;
  }
  g_cpu->m_ra.d[1] = 0x00000000;
  g_cpu->m_ra.d[0] = ret_addr;
  g_cpu->m_a0.d[0] = p_args;
  g_cpu->m_pc = p_func_va;
  
  int idx  = VA2IDX(g_cpu->m_pc);
  u8* addr = g_cpu->m_recompiled_code_table[idx];
  if (addr == NULL) {
    // 一旦実行ファイバに遷移してリコンパイルする
    SwitchToFiber(g_app.execute_fiber());
    addr = g_cpu->m_recompiled_code_table[idx];
  }
  while (1) {
    _asm call addr;
    // 未コンパイルコードか、ブレークポイントを検出した場合のみ
    // retで関数を抜けるので実行ファイバに遷移する
    SwitchToFiber(g_app.execute_fiber());
    idx  = VA2IDX(g_cpu->m_pc);
    addr = g_cpu->m_recompiled_code_table[idx];
  }

  // コールバック呼び出し前にraに指定したアドレスが
  // jr命令に処理されるとこのラベルへ飛ぶ
  _asm {
_ret:
    pop eax; // callで飛んでjmpで戻ってくるのでここでスタック調整
    popad;   // レジスタ復元
  }

  //*(R5900REGS*)g_cpu = save_regs;
  g_cpu->m_ra.q[0] = save_ra;
  g_cpu->m_v0.q[0] = save_v0;
  g_cpu->m_pc = save_pc;
}

u32* Device::getCharaPaletteAddr(u32 p_chara_mode, u32 p_chara_id) {
  return (u32*)VA2PA(ps2_pal_addr_table_[p_chara_mode][p_chara_id]);
}

void Device::activate() {
  sound_mgr_->activate();
  input_mgr_->activate();
}

bool Device::isRenderingNow() {
  return graph_mgr_->isRenderingNow();
}

#if !defined _DEBUGGER
void Device::toggleScreenMode() {
  graph_mgr_->toggleScreenMode();
}

void Device::changeWindowMode() {
  graph_mgr_->changeWindowMode();
}

void Device::restoreDevice() {
  graph_mgr_->restoreDevice();
}
#endif // !defined _DEBUGGER

DWORD CALLBACK vbl_proc(LPVOID lpParameter) {
  g_vbl_thread_end = false;
  while (g_vbl_thread_end == false) {
    const f64 kInterval = 16.66667;
    u32 now = timeGetTime();
    f64 diff = (f64)now - (f64)g_dev_emu->vbl_time0_
                        - kInterval + g_dev_emu->vbl_diff_;
    if (diff < 0.0) {
      Sleep(1);
    } else {
      g_dev_emu->vbl_diff_ = diff;
      g_dev_emu->vbl_time0_ = now;
      g_dev_emu->vbl_count_++;
      //ds_util::DBGOUTA("diff=%.5f(ms)\n%.2f fps\n",
      //                 diff, 1000.0 / (kInterval + diff));
    }
  }
  g_vbl_thread_end = false;
  return 0;
}

bool Device::initFileLoadThread() {
  file_load_thread_end_ = false;
  DWORD tid;
  file_load_thread_hdl_ = CreateThread(NULL, 0,
    fileLoadThreadProc, this, CREATE_SUSPENDED, &tid);
  if (file_load_thread_hdl_ == NULL) return false;

  SetThreadPriority(file_load_thread_hdl_, THREAD_PRIORITY_BELOW_NORMAL);
  ResumeThread(file_load_thread_hdl_);
  return true;
}

void Device::cleanFileLoadThread() {
  if (file_load_thread_hdl_) {
    file_load_thread_end_ = true;
    // ファイルロード終了するまで待機
    WaitForSingleObject(file_load_thread_hdl_, 2000);
    
    CloseHandle(file_load_thread_hdl_);
    file_load_thread_hdl_ = NULL;
  }
}

void Device::initInputMgr() {
  input_mgr_ = new InputMgr();
  input_mgr_->init(GetModuleHandle(NULL));
}

void Device::cleanInputMgr() {
  delete input_mgr_;
  input_mgr_ = NULL;
}

eProduct Device::getProduct(s8* p_product_cd) {
  static const struct {
    s8       str[16];
    eProduct value;
  } table[] = {
    "SLPS_201.37", kProduct_GGX,
    "SLPS_251.84", kProduct_GGXX,
    "SLPS_252.61", kProduct_GGXXR,
    "SLPM_656.44", kProduct_GGISUKA,
    "SLPM_663.33", kProduct_GGXXS,
    "SLPM_669.65", kProduct_GGXXACP,
    "",            kProduct_Invalid,
  };
  for (s32 i = 0; table[i].value != kProduct_Invalid; i++) {
    if (strcmp(p_product_cd, table[i].str) == 0) return table[i].value;
  }
  return kProduct_Invalid;
}

void Device::bindPs2Address(s8* p_product_cd) {
  if (getProduct(p_product_cd) == kProduct_GGX) {
    strcpy(product_name_, "Guilty Gear X Plus");
  } else if (getProduct(p_product_cd) == kProduct_GGXX) {
    strcpy(product_name_, "Guilty Gear XX");
    ps2_img_ref_data_          = (ImageRefData*)VA2PA(0x260708);
    ps2_max_img_ref_count_     = 2100;
    ps2_render_mode_           = (u32*)VA2PA(0x284ca0);
    ps2_clear_color_           = (u32*)VA2PA(0x282444);
    ps2_chara_boi_va_          = (u32*)VA2PA(0x28da70); 
    ps2_drawing_boi_va_        = (u32*)VA2PA(0x010000);
    ps2_chara_pal_id_1byte_    = (u8*)VA2PA(0x289c9a);
    ps2_dynamic_pal_addr_[0]   = (u32*)VA2PA(0x281440);
    ps2_dynamic_pal_addr_[1]   = (u32*)VA2PA(0x281840);
    ps2_pal_addr_table_[0]     = (u32*)VA2PA(0x28cf34);
    ps2_pal_addr_table_[1]     = (u32*)VA2PA(0x28d226);
    ps2_character_mode_[0]     = (u32*)VA2PA(0x289d40);
    ps2_character_mode_[1]     = (u32*)VA2PA(0x289d44);
    ps2_file_target_           = (FileMgmtTab*)VA2PA(0x38ac58);
    ps2_file_busy_             = (u32*)VA2PA(0x38acf0);
    ps2_movie_dst_buf_ptr_     = (u32*)VA2PA(0x38f364);
    ps2_movie_dst_buf_size_    = (u32*)VA2PA(0x38f368);
    ps2_movie_src_buf_ptr_     = (u32*)VA2PA(0x38f350);
    ps2_movie_src_buf_size_    = (u32*)VA2PA(0x38f354);
    ps2_movie_buf_unproc_size_ = (u32*)VA2PA(0x38f21c);
    ps2_movie_buf_proc_size_   = (u32*)VA2PA(0x38f22c);
    ps2_movie_buf_proc_pos_    = (u32*)VA2PA(0x38f218);
  } else if (getProduct(p_product_cd) == kProduct_GGXXR) {
    strcpy(product_name_, "Guilty Gear XX #Reload");
    ps2_img_ref_data_          = (ImageRefData*)VA2PA(0x26b388);
    ps2_max_img_ref_count_     = 2100;
    ps2_render_mode_           = (u32*)VA2PA(0x28f920);
    ps2_clear_color_           = (u32*)VA2PA(0x28d0c4);
    ps2_chara_boi_va_          = (u32*)VA2PA(0x297730);
    ps2_drawing_boi_va_        = (u32*)VA2PA(0x010000);
    ps2_chara_pal_id_          = (u16*)VA2PA(0x294a5c);
    ps2_dynamic_pal_addr_[0]   = (u32*)VA2PA(0x28c0c0);
    ps2_dynamic_pal_addr_[1]   = (u32*)VA2PA(0x28c4c0);
    ps2_pal_addr_table_[0]     = (u32*)VA2PA(0x297d54);
    ps2_pal_addr_table_[1]     = (u32*)VA2PA(0x298058);
    ps2_character_mode_[0]     = (u32*)VA2PA(0x294b3c);
    ps2_character_mode_[1]     = (u32*)VA2PA(0x294b40);
    ps2_file_target_           = (FileMgmtTab*)VA2PA(0x3b3dd8);
    ps2_file_busy_             = (u32*)VA2PA(0x3b3e70);
    ps2_movie_dst_buf_ptr_     = (u32*)VA2PA(0x3b84e4);
    ps2_movie_dst_buf_size_    = (u32*)VA2PA(0x3b84e8);
    ps2_movie_src_buf_ptr_     = (u32*)VA2PA(0x3b84d0);
    ps2_movie_src_buf_size_    = (u32*)VA2PA(0x3b84d4);
    ps2_movie_buf_unproc_size_ = (u32*)VA2PA(0x3b839c);
    ps2_movie_buf_proc_size_   = (u32*)VA2PA(0x3b83ac);
    ps2_movie_buf_proc_pos_    = (u32*)VA2PA(0x3b8398);
  } else if (getProduct(p_product_cd) == kProduct_GGISUKA) {
    strcpy(product_name_, "Guilty Gear Isuka");
  } else if (getProduct(p_product_cd) == kProduct_GGXXS) {
    strcpy(product_name_, "Guilty Gear XX Slash");
    ps2_img_ref_data_          = (ImageRefData*)VA2PA(0x2a6790);
    ps2_max_img_ref_count_     = 2200;
    ps2_render_mode_           = (u32*)VA2PA(0x2cc620);
    ps2_clear_color_           = (u32*)VA2PA(0x2c9dc4);
    ps2_chara_boi_va_          = (u32*)VA2PA(0x2f1b48); 
    ps2_drawing_boi_va_        = (u32*)VA2PA(0x36becc);
    ps2_chara_pal_id_          = (u16*)VA2PA(0x33a3d4);
    ps2_dynamic_pal_addr_[0]   = (u32*)VA2PA(0x2c8dc0);
    ps2_dynamic_pal_addr_[1]   = (u32*)VA2PA(0x2c91c0);
    ps2_pal_addr_table_[0]     = (u32*)VA2PA(0x3d00fc);
    ps2_pal_addr_table_[1]     = (u32*)VA2PA(0x3d0424);
    ps2_pal_addr_table_[2]     = (u32*)VA2PA(0x3d074c);
    ps2_character_mode_[0]     = (u32*)VA2PA(0x33a4b4);
    ps2_character_mode_[1]     = (u32*)VA2PA(0x33a4b8);
    ps2_file_target_           = (FileMgmtTab*)VA2PA(0x3b10d8);
    ps2_file_busy_             = (u32*)VA2PA(0x3b1170);
    ps2_movie_dst_buf_ptr_     = (u32*)VA2PA(0x3b57e4);
    ps2_movie_dst_buf_size_    = (u32*)VA2PA(0x3b57e8);
    ps2_movie_src_buf_ptr_     = (u32*)VA2PA(0x3b57d0);
    ps2_movie_src_buf_size_    = (u32*)VA2PA(0x3b57d4);
    ps2_movie_buf_unproc_size_ = (u32*)VA2PA(0x3b569c);
    ps2_movie_buf_proc_size_   = (u32*)VA2PA(0x3b56ac);
    ps2_movie_buf_proc_pos_    = (u32*)VA2PA(0x3b5698);
  } else if (getProduct(p_product_cd) == kProduct_GGXXACP) {
    strcpy(product_name_, "Guilty Gear XX Accent Core Plus");
    ps2_img_ref_data_          = (ImageRefData*)VA2PA(0x3067a0);
    ps2_max_img_ref_count_     = 2200;
    ps2_render_mode_           = (u32*)VA2PA(0x32cd00);
    ps2_clear_color_           = (u32*)VA2PA(0x329dc4);
    ps2_chara_boi_va_          = (u32*)VA2PA(0x352080);
    ps2_drawing_boi_va_        = (u32*)VA2PA(0x400f34);
    ps2_chara_pal_id_          = (u16*)VA2PA(0x3c4c5c);
    ps2_dynamic_pal_addr_[0]   = (u32*)VA2PA(0x328dc0);
    ps2_dynamic_pal_addr_[1]   = (u32*)VA2PA(0x3291c0);
    ps2_pal_addr_table_[0]     = (u32*)VA2PA(0x47179c);
    ps2_pal_addr_table_[1]     = (u32*)VA2PA(0x471ac4);
    ps2_character_mode_[0]     = (u32*)VA2PA(0x3c4d40);
    ps2_character_mode_[1]     = (u32*)VA2PA(0x3c4d44);
    ps2_file_target_           = (FileMgmtTab*)VA2PA(0x4523d8);
    ps2_file_busy_             = (u32*)VA2PA(0x452470);
    ps2_movie_dst_buf_ptr_     = (u32*)VA2PA(0x456ae4);
    ps2_movie_dst_buf_size_    = (u32*)VA2PA(0x456ae8);
    ps2_movie_src_buf_ptr_     = (u32*)VA2PA(0x456ad0);
    ps2_movie_src_buf_size_    = (u32*)VA2PA(0x456ad4);
    ps2_movie_buf_unproc_size_ = (u32*)VA2PA(0x45699c);
    ps2_movie_buf_proc_size_   = (u32*)VA2PA(0x4569ac);
    ps2_movie_buf_proc_pos_    = (u32*)VA2PA(0x456998);
  }
}

DWORD WINAPI Device::fileLoadThreadProc(LPVOID p_param) {
  Device* dev_emu = (Device*)p_param;

  dev_emu->file_load_thread_end_ = false;
  while (dev_emu->file_load_thread_end_ == false) {
    if (dev_emu->ps2_file_busy_ && dev_emu->ps2_file_target_ &&
        *dev_emu->ps2_file_busy_ == 0 &&
        dev_emu->ps2_file_target_->id >= 0 &&
        dev_emu->ps2_file_target_->buffer) {
      *dev_emu->ps2_file_busy_ = 1;
      s8* buf = (s8*)VA2PA(dev_emu->ps2_file_target_->buffer);
      int offset = dev_emu->ps2_file_target_->file_offset << 11;
      int size = dev_emu->ps2_file_target_->file_size << 11;

      s8 fpath[256];
      sprintf(fpath, "%s%s", g_elf_dir, dev_emu->ps2_file_target_->path);
      fpath[strlen(fpath) - 2] = '\0';
      
      DBGOUT_CPU("%s idx=%d buf=0x%x file=%s ofs=0x%x size=0x%x\n",
                 __FUNCTION__, dev_emu->ps2_file_target_->id,
                 dev_emu->ps2_file_target_->buffer, fpath, offset, size);

      FILE* fp = fopen(fpath, "rb");
      if (fp == NULL) {
        DBGOUT_CPU("cannot open file!! file=%s\n", fpath);
        continue;
      }
      fseek(fp, offset, SEEK_SET);
      fread(buf, size, 1, fp);
      fclose(fp);

      dev_emu->ps2_file_target_->id = -1;
      *dev_emu->ps2_file_busy_ = 0;
    }
    Sleep(50);
  }
  dev_emu->file_load_thread_end_ = false;
  return 0;
}
