#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")

#include "main.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "device/device.h"
#include "device/setting.h"
#include "device/audio.h"
#if defined _DEBUGGER
#include "debugger/debugger.h"
#include "debugger/gui/window/screen_window.h"
#else  // defined _DEBUGGER
#include "window.h"
#endif // defined _DEBUGGER
#include <math.h>
#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

//extern SettingFileMgr* setting_file_mgr_;

SoundDriver::SoundDriver() :
    ds8_(NULL),
    primary_dsb_(NULL),
    se_dsb_ring_pos_(0),
    se_volume_(0),
    voice_volume_(0),
    bgm_dsb_(NULL),
    last_play_buf_idx_(-1),
    bgm_volume_(0),
    bgm_position_(0),
    bgm_buf_va_(0),
    bgm_buf_size_(0),
    bgm_buf_page_size_(0),
    bgm_data_size_in_page_(0),
    bgm_playing_(false),
    shortbgm_dsb_(NULL),
    snd_mem_(NULL),
    cmn_se_addr_(0x00000000),
    chr_yy_addr_(0x00000000),
    chr_sy_addr_(0x00000000),
    cmn_se_max_(0),
    chr_yy_max_(0),
    chr_sy_max_(0) {
  // PCM用のサウンドフォーマット
  memset(&pcmwf_, 0, sizeof(pcmwf_));
  pcmwf_.wFormatTag      = WAVE_FORMAT_PCM;
  pcmwf_.nChannels       = 2;
  pcmwf_.nSamplesPerSec  = 48000;
  pcmwf_.wBitsPerSample  = 16;
  pcmwf_.nBlockAlign     = pcmwf_.nChannels * pcmwf_.wBitsPerSample / 8;
  pcmwf_.nAvgBytesPerSec = pcmwf_.nSamplesPerSec * pcmwf_.nBlockAlign;
  pcmwf_.cbSize          = 0;

  for (int i = 0; i < kSoundRingNum; i++) {
    se_dsb_ring_buf_[i] = NULL;
  }
  voice_dsb_[0] = NULL;
  voice_dsb_[1] = NULL;
  shortbgm_buf_va_[0] = NULL;
  shortbgm_buf_va_[1] = NULL;
  shortbgm_buf_size_[0] = NULL;
  shortbgm_buf_size_[1] = NULL;

  chr_se_addr_[0] = 0x00000000;
  chr_se_addr_[1] = 0x00000000;
  chr_vo_addr_[0] = 0x00000000;
  chr_vo_addr_[1] = 0x00000000;
  chr_se_max_[0] = 0;
  chr_se_max_[1] = 0;
  chr_vo_max_[0] = 0;
  chr_vo_max_[1] = 0;
}

bool SoundDriver::init() {
  HRESULT      hr;
  DSBUFFERDESC dsdesc;

  // 仮実装。本来はIOPメモリに持つべき
  snd_mem_ = new u8[0x200000];

  hr = DirectSoundCreate8(NULL, &ds8_, NULL);
  if (FAILED(hr)) {
    g_scrn->errMsgBox(kErr_FailedInitDSound, hr);
    return false;
  }

  hr = ds8_->SetCooperativeLevel(g_scrn->hwnd(), DSSCL_PRIORITY);
  if (FAILED(hr)) {
    g_scrn->errMsgBox(kErr_CallCOMFunc, "SetCooperativeLevel", hr);
    return false;
  }

  // プライマリバッファの作成
  memset(&dsdesc, 0, sizeof(DSBUFFERDESC));
  dsdesc.dwSize = sizeof(DSBUFFERDESC);
  dsdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
  dsdesc.dwBufferBytes = 0;
  hr = ds8_->CreateSoundBuffer(&dsdesc, &primary_dsb_, NULL);
  if (FAILED(hr)) {
    g_scrn->errMsgBox(kErr_CallCOMFunc, "CreateSoundBuffer", hr);
    return false;
  }
  primary_dsb_->SetFormat(&pcmwf_);

  return true;
}

void SoundDriver::clean() {
  clear_bgm();

  // 各種バッファを破棄
  for (int i = 0; i < kSoundRingNum; i++) {
    if (se_dsb_ring_buf_[i]) {
      se_dsb_ring_buf_[i]->Release();
      se_dsb_ring_buf_[i] = NULL;
    }
  }
  voice_dsb_[0] = NULL;
  voice_dsb_[1] = NULL;
  shortbgm_dsb_ = NULL;

  bgm_dsb_ = NULL;

  // プライマリバッファを破棄
  if (primary_dsb_) {
    primary_dsb_->Release();
    primary_dsb_ = NULL;
  }
  // DSOUNDオブジェクトを破棄
  if (ds8_) {
    ds8_->Release();
    ds8_ = NULL;
  }
  if (snd_mem_) {
    delete[] snd_mem_;
    snd_mem_ = NULL;
  }
}

bool SoundDriver::setup_bgm() {
  HRESULT  hr;
  if (ds8_ == NULL) return false;

  clear_bgm();

  // ストリーミング用セカンダリバッファの作成
  DSBUFFERDESC dsdesc;
  memset(&dsdesc, 0, sizeof(DSBUFFERDESC));
  dsdesc.dwSize  = sizeof(DSBUFFERDESC);
  dsdesc.dwFlags = DSBCAPS_LOCSOFTWARE |
                   DSBCAPS_GETCURRENTPOSITION2 |
                   DSBCAPS_CTRLVOLUME |
                   DSBCAPS_GLOBALFOCUS;
  dsdesc.dwBufferBytes = bgm_buf_size_;
  dsdesc.lpwfxFormat   = &pcmwf_;

  LPDIRECTSOUNDBUFFER dsb;
  hr = ds8_->CreateSoundBuffer(&dsdesc, &dsb, NULL);
  if (FAILED(hr)) {
    g_scrn->errMsgBox(kErr_CallCOMFunc, "CreateSoundBuffer", hr);
    return false;
  }

  hr = dsb->QueryInterface(IID_IDirectSoundBuffer8, (void**)&bgm_dsb_);
  if (FAILED(hr)) {
    g_scrn->errMsgBox(kErr_CallCOMFunc, "QueryInterface", hr);
    return false;
  }

  dsb->Release();

  // 音量設定
  bgm_playing_ = false;
  bgm_data_size_in_page_ = 0;
  fill_bgm_buffer(0);
  fill_bgm_buffer(1);

  return true;
}

void SoundDriver::clear_bgm() {
  if (bgm_dsb_) {
    bgm_dsb_->Stop();
    bgm_dsb_->Release();
    bgm_dsb_ = NULL;
  }
}

void SoundDriver::play_voice(u8* p_enc_data1, int p_size, int p_side) {
  play_adpcm(&voice_dsb_[p_side],
    p_enc_data1,
    NULL,
    p_size,
    voice_volume_ / 128,
    1,
    12000);
}

void SoundDriver::play_sound(u8* p_enc_data1, int p_size) {
  play_adpcm(&se_dsb_ring_buf_[se_dsb_ring_pos_],
    p_enc_data1,
    NULL,
    p_size,
    se_volume_ / 128,
    1,
    12000);
  se_dsb_ring_pos_ = (se_dsb_ring_pos_ + 1) % kSoundRingNum;
}

void SoundDriver::stop_all_sounds() {
  for (int i = 0; i < kSoundRingNum; i++) {
    if (se_dsb_ring_buf_[i]) {
      se_dsb_ring_buf_[i]->Stop();
      se_dsb_ring_buf_[i]->Release();
      se_dsb_ring_buf_[i] = NULL;
    }
  }
  se_dsb_ring_pos_ = 0;

  for (int i = 0; i < 2; i++) {
    if (voice_dsb_[i]) {
      voice_dsb_[i]->Stop();
      voice_dsb_[i]->Release();
      voice_dsb_[i] = NULL;
    }
  }
}

void SoundDriver::play_shortbgm(u8* p_enc_dataL, u8* p_enc_dataR, int p_size) {
  play_adpcm(&shortbgm_dsb_,
    p_enc_dataL,
    p_enc_dataR,
    p_size,
    bgm_volume_ / 256,
    2,
    48000);
}

void SoundDriver::stop_shortbgm() {
  if (shortbgm_dsb_) {
    shortbgm_dsb_->Stop();
    shortbgm_dsb_->Release();
    shortbgm_dsb_ = NULL;
  }
}

void SoundDriver::activate() {
  if (g_app.setting_file_mgr() &&
      g_app.setting_file_mgr()->get_data()->play_in_background() == false) {
    u32 vol = (bgm_playing_ ? bgm_volume_ : 0);
    long db = vol2db(g_scrn->active() ? vol / 256 : 0);
    if (bgm_dsb_) bgm_dsb_->SetVolume(db);
    if (shortbgm_dsb_) shortbgm_dsb_->SetVolume(db);
  }
}

void SoundDriver::call_snddrv_rpc(
    u32 p_client_data_ptr,
    u32 p_fno,
    u32 p_send_buf_ptr,
    u32 p_send_buf_size,
    u32 p_recv_buf_ptr,
    u32 p_recv_buf_size) {
  u32* args = (u32*)VA2PA(g_cpu->m_v0.d[0]);
  switch (p_fno) {
    case 0x00:
      DBGOUT_CPU("SifCallRpc Sound Invalid FunctionID!!\n");
      break;
    case 0x01:
    case 0x02:
      DBGOUT_CPU("SifCallRpc Sound 0x%02x Driver Startup\n", p_fno);
      cmn_se_addr_    = args[3];
      chr_se_addr_[0] = args[4];
      chr_se_addr_[1] = args[5];
      chr_yy_addr_    = args[6];
      chr_vo_addr_[0] = args[11];
      chr_vo_addr_[1] = args[12];
      chr_sy_addr_    = args[14];
      break;
    case 0x03:
      DBGOUT_CPU("SifCallRpc Sound 0x%02x SeInit\n", p_fno);
      break;
    case 0x04:
      assert(0);
      break;
    case 0x05: {
      const u8 chan[6][8] = {
        "Cmn", "ChVo", "ChSe", "ChYY", "ChSY", "ChEx",
      };
      DBGOUT_CPU("SifCallRpc Sound 0x%02x PlayId=%s "
                 "side=%dP id=%d cmd_idx=%d\n",
                 p_fno, chan[args[1]], args[0] + 1, args[2], args[4]);
      // 引数に指定されたIDからバッファとサイズを算出する
      u32* src_buf;
      switch (args[1]) {
        case 0: // common se
          src_buf = (u32*)(snd_mem_ + cmn_se_addr_);
          break;
        case 1: // chara vo 
          assert(0); // (fno=0x06で再生されるのでここにはこないはず)
          break;
        case 2: // chara se
          src_buf = (u32*)(snd_mem_ + chr_se_addr_[args[0]]);
          break;
        case 3: // chara yy
          src_buf = (u32*)(snd_mem_ + chr_yy_addr_);
          break;
        case 4: // chara sy
          assert(0); // (fno=0x06で再生されるのでここにはこないはず)
          break;
        case 5: // chara ex
          assert(0); // ???
          break;
      }
      if (src_buf == NULL) break;

      int snd_num     = src_buf[0];
      int header_size = 32 + (((snd_num - 2) * 4 + 15) & 0xfffffff0);
      u8* snd_buf     = (u8*)src_buf + header_size + src_buf[args[2] + 1];
      int snd_size    = src_buf[args[2] + 2] - src_buf[args[2] + 1];

      if (args[1] == 1 || args[1] == 4) {
        play_voice(snd_buf, snd_size, args[0]);
      } else {
        play_sound(snd_buf, snd_size);
      }
    } break;
    case 0x06: {
      DBGOUT_CPU("SifCallRpc Sound 0x%02x PlayBuffer side=%dP id=%d "
                 "buf=0x%08x size=0x%08x cmd_idx=%d\n",
                 p_fno, args[0] + 1, args[2], args[5], args[6], args[4]);
      if (args[5] == NULL || args[6] == 0) {
        // Sol,Dizzy,Slayer,Order-Solの４体は攻撃Lv1のガードボイスが
        // 存在しないが再生されているためここでチェックしないと落ちる
        break;
      }
      // 引数にてバッファのポインタとサイズが既に与えられている
      u8* snd_buf  = (u8*)VA2PA(args[5]);
      int snd_size = args[6];

      if (args[1] == 1 || args[1] == 4) {
        play_voice(snd_buf, snd_size, args[0]);
      } else {
        play_sound(snd_buf, snd_size);
      }
    } break;
    case 0x07:
      assert(0);
      break;
    case 0x08:
      assert(0);
      break;
    case 0x09:
      assert(0);
      break;
    case 0x0a:
      DBGOUT_CPU("SifCallRpc Sound 0x%02x SeAllStopByKind\n", p_fno);
      break;
    case 0x0b:
      DBGOUT_CPU("SifCallRpc Sound 0x%02x SeAllStop\n", p_fno);
      stop_all_sounds();
      break;
    case 0x0c:
      assert(0);
      break;
    case 0x0d:
      DBGOUT_CPU("SifCallRpc Sound 0x%02x SetSeVoAttrImmediate\n", p_fno);
      break;
    case 0x0e:
      DBGOUT_CPU("SifCallRpc Sound 0x%02x SetSeVoVolImmediate\n", p_fno);
      break;
    case 0x0f:
      DBGOUT_CPU("SifCallRpc Sound 0x%02x SeReadPre %d %d 0x%x\n",
                 p_fno, args[0], args[1], args[2]);
      *((u32*)VA2PA(p_recv_buf_ptr +  0)) = -1;
      break;
    case 0x10:
      DBGOUT_CPU("SifCallRpc Sound 0x%02x SeRead %d %d\n",
                 p_fno, args[0], args[1]);
      switch (args[1]) {
        case 0: 
          memcpy(snd_mem_ + cmn_se_addr_,
                 (void*)VA2PA(args[4]),
                 get_sound_bank_size(args[4]));
          cmn_se_max_ = get_sound_bank_entry_count(args[4]);
          DBGOUT_CPU("\tCmnSeMax count=%d (buf=0x%x->0x%x)\n",
                     cmn_se_max_, args[4], cmn_se_addr_);
          *((u32*)VA2PA(p_recv_buf_ptr +  0)) = cmn_se_max_;
          break;
        case 1:
          DBGOUT_CPU("\tChrVoMax %dP count=%d\n", args[0], args[7]);
          chr_vo_max_[args[0]] = args[7];
          *((u32*)VA2PA(p_recv_buf_ptr +  0)) = chr_vo_max_[args[0]];
          break;
        case 2: {
          u32 buf = args[6] ? args[6] : args[4];
          memcpy(snd_mem_ + chr_se_addr_[args[0]],
                 (void*)VA2PA(buf),
                 get_sound_bank_size(args[4]));
          chr_se_max_[args[0]] = get_sound_bank_entry_count(buf);
          DBGOUT_CPU("\tChrSeMax %dP count=%d (buf=0x%x->0x%x)\n",
                     args[0] + 1, chr_se_max_, buf, chr_se_addr_[args[0]]);
          *((u32*)VA2PA(p_recv_buf_ptr +  0)) = chr_se_max_[args[0]];
        } break;
        case 3:
          memcpy(snd_mem_ + chr_yy_addr_,
                 (void*)VA2PA(args[4]),
                 get_sound_bank_size(args[4]));
          chr_yy_max_ = get_sound_bank_entry_count(args[4]);
          DBGOUT_CPU("\tChrYYMax count=%d (buf=0x%x->0x%x)\n",
                     chr_yy_max_, args[4], chr_yy_addr_);
          *((u32*)VA2PA(p_recv_buf_ptr +  0)) = chr_yy_max_;
          break;
        case 4:
          chr_sy_max_ = args[7];
          DBGOUT_CPU("\tChrSeSYMax %d\n", args[7]);
          *((u32*)VA2PA(p_recv_buf_ptr +  0)) = chr_sy_max_;
          break;
        case 5: // 未使用？
          assert(0);
          break;
      }
      break;
    case 0x11:
      DBGOUT_CPU("SifCallRpc Sound 0x%02x SetSeVol %d\n", p_fno, args[0]);
      se_volume_ = args[0];
      break;
    case 0x12:
      DBGOUT_CPU("SifCallRpc Sound 0x%02x SetVoVol %d\n", p_fno, args[0]);
      voice_volume_ = args[0];
      break;
    case 0x8004:
      *((u32*)VA2PA(p_recv_buf_ptr +  4)) = cmn_se_max_;
      *((u32*)VA2PA(p_recv_buf_ptr +  8)) = chr_vo_max_[0];
      *((u32*)VA2PA(p_recv_buf_ptr + 16)) = chr_se_max_[0];
      *((u32*)VA2PA(p_recv_buf_ptr + 24)) = chr_yy_max_;
      *((u32*)VA2PA(p_recv_buf_ptr + 28)) = chr_sy_max_;
      break;
  }
}

void SoundDriver::call_isdsrv_rpc(
    u32 p_client_data_ptr,
    u32 p_fno,
    u32 p_send_buf_ptr,
    u32 p_send_buf_size,
    u32 p_recv_buf_ptr,
    u32 p_recv_buf_size) {
  u32* args = (u32*)VA2PA(g_cpu->m_v0.d[0]);

  // 予期しない動作を避けるためクリアしておく
  if (p_recv_buf_size > 0) {
    memset((void*)VA2PA(p_recv_buf_ptr), 0, p_recv_buf_size);
  }

  switch (p_fno) {
    case 0x00:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x Init\n", p_fno);
      last_play_buf_idx_ = -1;
      bgm_position_ = 0;
      shortbgm_buf_va_[0] = NULL;
      shortbgm_buf_va_[1] = NULL;
      shortbgm_buf_size_[0] = 0;
      shortbgm_buf_size_[1] = 0;
      break;
    case 0x01: {
      DBGOUT_CPU("SifCallRpc Stream 0x%02x ADPCMSetAttr bufno=%d "
                 "arg2=0x%x dstbuf=0x%08x arg4=0x%x size=0x%x arg6=%d\n",
                 p_fno,
                 ((u32*)VA2PA(p_send_buf_ptr))[1],
                 ((u32*)VA2PA(p_send_buf_ptr))[2],
                 ((u32*)VA2PA(p_send_buf_ptr))[3],
                 ((u32*)VA2PA(p_send_buf_ptr))[4],
                 ((u32*)VA2PA(p_send_buf_ptr))[5],
                 ((u32*)VA2PA(p_send_buf_ptr))[6]);

      int idx = ((u32*)VA2PA(p_send_buf_ptr))[1];
      shortbgm_buf_va_[idx & 1] = ((u32*)VA2PA(p_send_buf_ptr))[3];
      shortbgm_buf_size_[idx & 1] = ((u32*)VA2PA(p_send_buf_ptr))[5];

      *((u32*)VA2PA(p_recv_buf_ptr +  0)) = 1;
    } break;
    case 0x02:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x\n", p_fno);
      break;
    case 0x03:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x arg1=%x\n",
                 p_fno, ((u32*)VA2PA(p_send_buf_ptr))[1]);
      break;
    case 0x04:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x\n", p_fno);
      break;
    case 0x05:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x\n", p_fno);
      break;
    case 0x06:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x ADPCM PrePlay side=%d\n",
                 p_fno, ((u32*)VA2PA(p_send_buf_ptr))[1]);
      break;
    case 0x07:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x ADPCM Play side=%d\n",
                 p_fno, ((u32*)VA2PA(p_send_buf_ptr))[1]);
      if (((u32*)VA2PA(p_send_buf_ptr))[1] == 0) {
        // ２回コマンドが来るのでside=0でのみ実行することにする
        play_shortbgm((u8*)VA2PA(shortbgm_buf_va_[0]),
                      (u8*)VA2PA(shortbgm_buf_va_[1]),
                      shortbgm_buf_size_[0]);
      }
      *((u32*)VA2PA(p_recv_buf_ptr +  0)) = 1;
      break;
    case 0x08:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x ADPCM CheckReady side=%d\n",
                 p_fno, ((u32*)VA2PA(p_send_buf_ptr))[1]);
      *((u32*)VA2PA(p_recv_buf_ptr +  0)) = 1;
      break;
    case 0x09:
      //DBGOUT_CPU("SifCallRpc Stream 0x%02x ADPCM Fetch1 side=%x\n",
      //           p_fno, ((u32*)VA2PA(p_send_buf_ptr))[1]);
      // 本来はデコードしたデータを返す。必要ないのでダミー
      *((u32*)VA2PA(p_recv_buf_ptr +  0)) = 0xcccccccc;
      break;
    case 0x0a:
      //DBGOUT_CPU("SifCallRpc Stream 0x%02x ADPCM Fetch2 side=%x\n",
      //           p_fno, ((u32*)VA2PA(p_send_buf_ptr))[1]);
      // 本来はデコードしたデータを返す。必要ないのでダミー
      *((u32*)VA2PA(p_recv_buf_ptr +  0)) = 0xcccccccc;
      break;
    case 0x0b:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x ADPCM Stop side=%x\n",
                 p_fno, ((u32*)VA2PA(p_send_buf_ptr))[1]);
      if (((u32*)VA2PA(p_send_buf_ptr))[1] == 0) {
        // ２回コマンドが来るのでside=0でのみ実行することにする
        stop_shortbgm();
      }
      *((u32*)VA2PA(p_recv_buf_ptr +  0)) = 0;
      break;
    case 0x0c:
      bgm_buf_va_   = ((u32*)VA2PA(p_send_buf_ptr))[1];
      bgm_buf_size_ = ((u32*)VA2PA(p_send_buf_ptr))[2];
      bgm_buf_page_size_ = bgm_buf_size_ / 2;
      DBGOUT_CPU("SifCallRpc Stream 0x%02x Setup va=0x%08x size=0x%x\n",
                 p_fno, bgm_buf_va_, bgm_buf_size_);
      setup_bgm();
      break;
    case 0x0d:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x ChangeVolume vol=%d arg2=%x\n",
                 p_fno,
                 ((u32*)VA2PA(p_send_buf_ptr))[1],
                 ((u32*)VA2PA(p_send_buf_ptr))[2]);
      bgm_volume_ = ((u32*)VA2PA(p_send_buf_ptr))[1];

      if (g_app.setting_file_mgr()->get_data()->play_in_background() ||
          g_scrn->active()) {
        if (bgm_dsb_) {
          u32 vol = (bgm_playing_ ? bgm_volume_ : 0);
          bgm_dsb_->SetVolume(vol2db(vol / 256));
        }
      }
      break;
    case 0x0e:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x MoveToHead\n", p_fno);
      if (bgm_dsb_) {
        bgm_dsb_->SetCurrentPosition(0);
      }
      break;
    case 0x0f: {
      int bufidx  = ((u32*)VA2PA(p_send_buf_ptr))[2];
      int size    = ((u32*)VA2PA(p_send_buf_ptr))[1];
      DBGOUT_CPU("SifCallRpc Stream 0x%02x FillBuffer buf[%d] size=0x%x\n",
                 p_fno, bufidx, size);
      if (size > 0) {
        fill_bgm_buffer(bufidx, size);
        bgm_data_size_in_page_ = size;
      } else {
        // バッファリング完了
        fill_bgm_buffer(bufidx);
        last_play_buf_idx_ = bufidx;
      }
    } break;
    case 0x10:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x\n", p_fno);
      break;
    case 0x11:
      bgm_buf_page_size_ = ((u32*)VA2PA(p_send_buf_ptr))[1];
      DBGOUT_CPU("SifCallRpc Stream 0x%02x SetBufferPageSize size=0x%x\n",
                 p_fno, bgm_buf_page_size_);
      break;
    case 0x12:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x Play\n", p_fno);
      last_play_buf_idx_ = -1;
      if (bgm_dsb_) {
        bgm_playing_ = true;
        if (g_app.setting_file_mgr()->get_data()->play_in_background() ||
            g_scrn->active()) {
          bgm_dsb_->SetVolume(vol2db(bgm_volume_ / 256));
        } else {
          bgm_dsb_->SetVolume(vol2db(0));
        }
        bgm_dsb_->Play(0, 0, DSBPLAY_LOOPING);
      }
      break;
    case 0x13:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x Return 1\n", p_fno);
      *((u32*)VA2PA(p_recv_buf_ptr +  0)) = 1;
      break;
    case 0x14:
      if (bgm_dsb_) {
        bgm_dsb_->GetCurrentPosition(&bgm_position_, NULL);
        *((u32*)VA2PA(p_recv_buf_ptr +  0)) = bgm_position_;
        DBGOUT_CPU("SifCallRpc Stream 0x%02x GetPcmPosition pos=%x\n",
                   p_fno, bgm_position_);
      }
      break;
    case 0x15: {
      s32  play_buf_idx = bgm_position_ < bgm_buf_page_size_ ? 0 : 1;
      bool play = false;
      if (last_play_buf_idx_ == -1 ||
          play_buf_idx != last_play_buf_idx_) {
        play = true;
      }
#if defined _DEBUG
      static bool last = false;
      if (play != last) {
        DBGOUT_CPU("SifCallRpc Stream 0x%02x "\
                   "IsPCMPlaying state changed %d > %d\n", p_fno, last, play);
        last = play;
      }
#endif // defined _DEBUG
      *((u32*)VA2PA(p_recv_buf_ptr +  0)) = play;
      if (last_play_buf_idx_ != -1 && play_buf_idx != last_play_buf_idx_) {
        u32 end_pos = bgm_data_size_in_page_ +
                      play_buf_idx * bgm_buf_page_size_;
        if (bgm_dsb_ && bgm_playing_ && bgm_position_ >= end_pos) {
          bgm_dsb_->SetVolume(vol2db(0));
          bgm_playing_ = false;
          DBGOUT_CPU("SifCallRpc Stream 0x%02x "\
                     "Play complete!! volume=0\n", p_fno);
        }
        DBGOUT_CPU("SifCallRpc Stream 0x%02x "\
                   "CurPos=%08x EndPos=%08x\n", p_fno, bgm_position_, end_pos);
      }
    } break;
    case 0x16:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x Clear Buffer\n", p_fno);
      bgm_dsb_->Stop();
      fill_bgm_buffer(0);
      fill_bgm_buffer(1);
      bgm_playing_ = false;
      *((u32*)VA2PA(p_recv_buf_ptr +  0)) = 0;
      break;
    default:
      DBGOUT_CPU("SifCallRpc Stream 0x%02x\n", p_fno);
      assert(0);
      break;
  }
}

int SoundDriver::get_sound_bank_entry_count(u32 p_va) {
  u32* pa_buf = (u32*)VA2PA(p_va);
  return pa_buf[0];
}

int SoundDriver::get_sound_bank_size(u32 p_va) {
  u32* pa_buf = (u32*)VA2PA(p_va);
  int snd_num     = pa_buf[0];
  int header_size = 32 + (((snd_num - 2) * 4 + 15) & 0xfffffff0);
  return header_size + pa_buf[snd_num + 1];
}

int SoundDriver::get_adpcm_decoded_size(int p_size) {
  // 実際には通常32byte程度見積もりより少なくなるが、
  // 正確なサイズは内部データを解析しなくてはいけないので多めに返す
  return (p_size >> 4) * 56;
}

int SoundDriver::decode_adpcm(u8* p_dst, u8* p_src1, u8* p_src2, int p_size) {
  const int ftab[2][5] = {
    0, 240,  460,  392,  488,
    0,   0, -208, -220, -240,
  };
  int old[2][2] = { 0, 0, 0, 0 };

  u16* dp    = (u16*)p_dst;
  u8*  sp[2] = { p_src1, p_src2 };
  int block_num = p_size >> (p_src2 ? 5 : 4);
  for (int i = 0; i < block_num; i++) {
    for (int n = 0; n < 2; n++) {
      if (sp[n] == NULL) break;

      u8 scale = 12 - (sp[n][0] & 0x0f);
      u8 fidx  = sp[n][0] >> 4;
      u8 code  = sp[n][1];

      for (int j = 0; j < 28; j++) {
        s32 val = ((sp[n][(j >> 1) + 2] >> ((j & 1) << 2)) & 0x0f);
        if (val & 0x08) val -= 0x10;
        val <<= scale;
        val += (old[n][0] * ftab[0][fidx] >> 8) +
               (old[n][1] * ftab[1][fidx] >> 8);
        old[n][1] = old[n][0];
        old[n][0] = val;

        if (val > 0x7fff)       val =  0x7fff;
        else if (val < -0x8000) val = -0x8000;
      
        *(dp + j * (p_src2 ? 2 : 1) + n) = (u16)val;
        //*(dp + j + n * 28) = (u16)val;
      }
      if (code == 1) {
        // データの終わり
        break;
      }
      sp[n] += 16;
    }
    dp += (p_src2 ? 56 : 28);
  }
  return (u32)dp - (u32)p_dst;
}

long SoundDriver::vol2db(int p_volume) {
  if (p_volume == 0) {
    return -10000;
  } else {
    return (long)(1000.0f * logf((float)p_volume / 160.0f));
  }
}


void SoundDriver::play_adpcm(LPDIRECTSOUNDBUFFER8* p_dsb8ptr,
    u8* p_enc_data1, u8* p_enc_data2,
    int p_size, int p_volume, int p_ch, int p_freq) {
  if (ds8_ == NULL) {
    return;
  }

  // 前のデータがあれば破棄
  if (*p_dsb8ptr) {
    (*p_dsb8ptr)->Stop();
    (*p_dsb8ptr)->Release();
    *p_dsb8ptr = NULL;
  }
  
  HRESULT hr;
  WAVEFORMATEX pcmwf;
  memset(&pcmwf, 0, sizeof(WAVEFORMATEX));
  pcmwf.cbSize          = sizeof(WAVEFORMATEX);
  pcmwf.wFormatTag      = WAVE_FORMAT_PCM;
  pcmwf.wBitsPerSample  = 16;
  pcmwf.nChannels       = p_ch;
  pcmwf.nSamplesPerSec  = p_freq;
  pcmwf.nBlockAlign     = pcmwf.nChannels * pcmwf.wBitsPerSample / 8;
  pcmwf.nAvgBytesPerSec = pcmwf.nBlockAlign * pcmwf.nSamplesPerSec;

  int dec_size = get_adpcm_decoded_size(p_size);

  // セカンダリバッファの作成
  DSBUFFERDESC  dsdesc;
  memset(&dsdesc, 0, sizeof(DSBUFFERDESC));
  dsdesc.dwSize  = sizeof(DSBUFFERDESC);
  dsdesc.dwFlags = DSBCAPS_LOCDEFER |
                   DSBCAPS_GETCURRENTPOSITION2 |
                   DSBCAPS_CTRLVOLUME;
  // 動的ボリュームコントロールしないのでオプションの有無で十分
  if (g_app.setting_file_mgr()->get_data()->play_in_background()) {
    dsdesc.dwFlags |= DSBCAPS_GLOBALFOCUS;
  }
  dsdesc.dwBufferBytes = dec_size;
  dsdesc.lpwfxFormat = &pcmwf;

  LPDIRECTSOUNDBUFFER8 dsb8;
  LPDIRECTSOUNDBUFFER  dsb;
  hr = ds8_->CreateSoundBuffer(&dsdesc, &dsb, NULL);
  assert(SUCCEEDED(hr));
  
  hr = dsb->QueryInterface(IID_IDirectSoundBuffer8, (void**)&dsb8);
  assert(SUCCEEDED(hr));

  dsb->Release();

  void  *ptr1, *ptr2;
  DWORD size1, size2;
  dsb8->Lock(0, 0, &ptr1, &size1, &ptr2, &size2, DSBLOCK_ENTIREBUFFER);
  
  decode_adpcm((u8*)ptr1, p_enc_data1, p_enc_data2, p_size);

  dsb8->Unlock(ptr1, size1, ptr2, size2);

  // 音量設定
  dsb8->SetVolume(vol2db(p_volume));
  dsb8->Play(0, 0, 0);

  *p_dsb8ptr = dsb8;
}

void SoundDriver::fill_bgm_buffer(s32 p_sector, u32 p_data_size) {
  HRESULT hr;
  if (ds8_ == NULL) return;

  s16 *addr1, *addr2;
  u32 size1, size2;
  hr = bgm_dsb_->Lock(bgm_buf_page_size_ * p_sector, bgm_buf_page_size_,
                      (void**)&addr1, &size1, (void**)&addr2, &size2, 0);
  assert(SUCCEEDED(hr));

  s16* src_addr = NULL;
  if (p_data_size > 0) {
    // 変換しつつバッファに格納
    s32 block_count = kPCMInterleave / 2;
    u32 samples = p_data_size >> 1;
    if (p_data_size < bgm_buf_page_size_) samples -= 0x800; // 末尾のノイズ回避
    src_addr = (s16*)VA2PA(bgm_buf_va_ + bgm_buf_page_size_ * p_sector);
    for (u32 i = 0; i < samples; i++) {
      s32 pos     = (i >> 1) % block_count;
      s32 channel = block_count * (i & 1);
      s32 block   = i / kPCMInterleave * kPCMInterleave;
      addr1[i] = *(src_addr + pos + channel + block);
    }
    // 余りがあれば0埋め
    if (p_data_size < bgm_buf_page_size_) {
      memset(addr1 + samples, 0, size1 - p_data_size);
    }
  } else {
    memset(addr1, 0, size1);
  }

#if 0
  // export extracted data dumps
  static int num = 0;
  s8 fname[1024];
  FILE* fp;
  sprintf(fname, "src%02x.dump", num);
  fp = fopen(fname, "wb");
  if (src_addr) {
    fwrite(src_addr, p_data_size, 1, fp);
  }
  fclose(fp);

  sprintf(fname, "dst%02x.dump", num);
  fp = fopen(fname, "wb");
  fwrite(addr1, bgm_buf_page_size_, 1, fp);
  fclose(fp);

  num++;
#endif

  hr = bgm_dsb_->Unlock(addr1, size1, addr2, size2);
  assert(SUCCEEDED(hr));
}
