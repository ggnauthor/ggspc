#ifndef _ggsext_audio
#define _ggsext_audio

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#include "main.h"

class SoundDriver {
public:
  enum {
    kPCMInterleave = 0x200,
    kSoundRingNum = 16,
  };

  SoundDriver();
  ~SoundDriver() {}

  bool init();
  void clean();

  bool setup_bgm();
  void clear_bgm();

  void play_voice(u8* p_enc_data1, int p_size, int p_side);
  void play_sound(u8* p_enc_data1, int p_size);
  void stop_all_sounds();

  void play_shortbgm(u8* p_enc_data1, u8* p_enc_data2, int p_size);
  void stop_shortbgm();

  void activate();

  void call_snddrv_rpc(u32 p_client_data_ptr, u32 p_fno,
                       u32 p_send_buf_ptr, u32 p_send_buf_size,
                       u32 p_recv_buf_ptr, u32 p_recv_buf_size);
  void call_isdsrv_rpc(u32 p_client_data_ptr, u32 p_fno,
                       u32 p_send_buf_ptr, u32 p_send_buf_size,
                       u32 p_recv_buf_ptr, u32 p_recv_buf_size);

  int get_sound_bank_entry_count(u32 p_va);
  int get_sound_bank_size(u32 p_va);

  int get_adpcm_decoded_size(int p_size);
  long vol2db(int p_volume);

#if !_UNIT_TEST
private:
#endif // _UNIT_TEST
  int decode_adpcm(u8* p_dst, u8* p_src1, u8* p_src2, int p_size);
  void play_adpcm(LPDIRECTSOUNDBUFFER8* p_dsb8ptr,
    u8* p_enc_data1, u8* p_enc_data2,
    int p_size, int p_volume, int p_ch, int p_freq);
  void fill_bgm_buffer(s32 p_sector, u32 p_data_size = 0);

  LPDIRECTSOUND8      ds8_;
  LPDIRECTSOUNDBUFFER primary_dsb_;

  WAVEFORMATEX pcmwf_;

  // ADPCM形式のSE用
  LPDIRECTSOUNDBUFFER8 se_dsb_ring_buf_[kSoundRingNum];
  int se_dsb_ring_pos_;
  u32 se_volume_; // MAX16383
  
  // ADPCM形式のVoice用
  LPDIRECTSOUNDBUFFER8 voice_dsb_[2];
  u32 voice_volume_; // MAX16383

  // PCM形式のBGM用(ストリーミング)
  LPDIRECTSOUNDBUFFER8 bgm_dsb_;
  s32  last_play_buf_idx_;
  u32  bgm_volume_; // MAX32767 shortbgmと兼用
  u32  bgm_position_;
  u32  bgm_buf_va_;
  u32  bgm_buf_size_;
  u32  bgm_buf_page_size_;
  u32  bgm_data_size_in_page_;
  bool bgm_playing_; // 再生開始から全データ再生まで
                     // PS2側に再生完了を返すのはページの再生完了時

  // ADPCM形式のBGM用(非ストリーミング)
  LPDIRECTSOUNDBUFFER8 shortbgm_dsb_;
  u32 shortbgm_buf_va_[2];
  u32 shortbgm_buf_size_[2];

  u8* snd_mem_;

  u32 cmn_se_addr_;
  u32 chr_se_addr_[2];
  u32 chr_yy_addr_;
  u32 chr_sy_addr_;
  u32 chr_vo_addr_[2];

  u32 cmn_se_max_;
  u32 chr_se_max_[2];
  u32 chr_yy_max_;
  u32 chr_sy_max_;
  u32 chr_vo_max_[2];
};

#endif // _ggsext_audio
