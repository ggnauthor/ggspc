#ifndef _ggsext_mpeg2dec
#define _ggsext_mpeg2dec

/*---------*/
/* include */
/*---------*/
#include "main.h"

/*--------*/
/* define */
/*--------*/

/*----------*/
/* function */
/*----------*/
class Mpeg2Decoder {
public:
  Mpeg2Decoder();
  ~Mpeg2Decoder();

#if defined _DEBUG
  void* operator new(size_t p_sz, int p_block, const char * p_file, int p_line) {
    void* ptr = _aligned_malloc_dbg(p_sz, 16, p_file, p_line);
    return ptr;
  }
  void operator delete(void* p_ptr) {
    _aligned_free_dbg(p_ptr);
  }
#else
  void* operator new(size_t p_sz) {
    void* ptr = _aligned_malloc(p_sz, 16);
    return ptr;
  }
  void operator delete(void* p_ptr) {
    _aligned_free(p_ptr);
  }
#endif

  int  fill_buffer(const u8* p_src_buf, int p_size);
  void reset();
  void get_pic(u32* p_dst_buf);

  inline bool comfirm_render() {
    bool result = m_available_render;
    m_available_render = false;
    return result;
  }
  inline u32  get_rgb_data(int p_x, int p_y) { return m_rgb_data[p_y * m_horizontal_size_value + p_x]; }

private:
  bool decode(u32* p_dst_buf);
  
  // sequence decode
  void sequence_decode(u8* &p_ptr, u32 &p_read_bit);

  // extension decode
  void sequence_extension_decode(u8* &p_ptr, u32 &p_read_bit);
  void sequence_display_extension_decode(u8* &p_ptr, u32 &p_read_bit);
  void sequence_scalable_extension_decode(u8* &p_ptr, u32 &p_src_bit);
  void picture_coding_extension_decode(u8* &p_ptr, u32 &p_src_bit);

  // gop decode
  void gop_decode(u8* &p_ptr, u32 &p_src_bit);
  
  // picture decode
  void picture_data_decode(u8* &p_ptr, u32 &p_src_bit);
  
  // slice decode
  void slice_decode(u8* &p_ptr, u32 &p_src_bit);
  
  // macro block decode
  void macro_block_decode(u8* &p_ptr, u32 &p_src_bit);
  void macro_block_render();
  void motion_vectors_decode(u8* &p_ptr, u32 &p_src_bit, int p_s);
  void motion_vector_decode(u8* &p_ptr, u32 &p_src_bit, int p_r, int p_s);
  void motion_vector_calc(bool p_fieldBasedPrediction);
  void coded_block_pattern_decode(u8* &p_ptr, u32 &p_src_bit);
  bool block_decode(u8* &p_ptr, u32 &p_src_bit, int p_i);
  void inverse_zigzag();
  void dequantize(int p_idx);
  void decode_dct(int p_idx);
  void clear_yuvdata(int p_idx);

  // idct 2d 8x8
  void idct2d_88(int* p_in, int* p_out);

private:
  bool  m_movie_end;
  bool  m_available_render;
  u8*    m_buf;
  u8*    m_ptr;          // 処理ポインタ
  u8*   m_read_ptr;     // 読込ポインタ

  u32    m_horizontal_size_value;
  u32    m_vertical_size_value;
  u32    m_aspect_ratio_information;
  u32    m_frame_rate_code;
  u32    m_bit_rate_value;
  u32    m_vbv_buffer_size_value;
  u8    m_intra_quantizer_matrix[64];
  u8    m_non_intra_quantizer_matrix[64];

  u32    m_profile_and_level_indication;
  u32    m_progressive_sequence;
  u32    m_chroma_format;
  u32    m_low_delay;

  u32    m_video_format;
  u32    m_color_primaries;
  u32    m_transfer_characteristics;
  u32    m_matrix_coefficients;
  u32    m_display_horizontal_size;
  u32    m_display_vertical_size;

  u32    m_scalable_extentions;
  u32    m_scalable_mode;
  u32    m_layer_id;
  u32    m_lower_layer_prediction_horizontal_size;
  u32    m_lower_layer_prediction_vertical_size;
  u32    m_horizontal_subsampling_factor_m;
  u32    m_horizontal_subsampling_factor_n;
  u32    m_vertical_subsampling_factor_m;
  u32    m_vertical_subsampling_factor_n;
  u32    m_mux_to_progressive_sequence;
  u32    m_picture_mux_order;
  u32    m_picture_mux_factor;

  u32    m_f_code[2][2];
  u32    m_intra_dc_precision;
  u32    m_picture_struct;
  u32    m_top_field_first;
  u32    m_frame_predictive_frame_dct;
  u32    m_concealment_motion_vectors;
  u32    m_q_scale_type;
  u32    m_intra_vlc_format;
  u32    m_alternate_scan;
  u32    m_repeat_first_field;
  u32    m_chroma_420_type;
  u32    m_progressive_frame;
  u32    m_v_axis;
  u32    m_field_sequence;
  u32    m_sub_carrier;
  u32    m_burst_amplitude;
  u32    m_sub_carrier_phese;

  u32    m_time_code;
  u32    m_closed_gop;
  u32    m_broken_link;

  u32    m_temporal_reference;
  u32    m_picture_coding_type;
  u32    m_vbv_delay;
  u32    m_full_pel_forward_vector;
  u32    m_forward_f_code;
  u32    m_full_pel_backward_vector;
  u32    m_backward_f_code;

  u32    m_slice_vertical_position_extension;
  u32    m_priority_breakpoint;
  u32    m_quantizer_scale_code;
  u32    m_intra_slice;

  u32    m_macroblock_address_increment;
  u32    m_mb_type;
  u32    m_frame_motion_type;
  u32    m_field_motion_type;
  u32    m_dual_prime;
  u32    m_mv_format;
  u32    m_motion_vector_count;

  s32    m_dct_type;

  u32    m_motion_vertical_field_select[2][2];  // 参照ピクチャのTop/Bottomフィールドのどちらか？
  s32    m_motion_code[2][2][2];          // 予測ベクトル
  u32    m_motion_residual[2][2][2];        // 残差ベクトル
  u32    m_dmvector[2];              // 差分動きベクトル
  s32    m_motion_vector[2][2][2];        // 動きベクトル

  u32    m_pattern_code[12];
  s32    m_dct_dc_differential;

  s32    m_qfs[64];
  s32    m_dct_dc_pred[3];

  s32    m_qf[8][8];
  s32    m_f[8][8];
  __declspec(align(16)) s32  m_yuv[3][16][16];

  u32*  m_rgb_data;
  u32    m_rgb_pos_x;
  u32    m_rgb_pos_y;

  // 被参照ピクチャ保持用のバッファ
  s32*  m_wk_pic[4];

  // 被参照ピクチャの数
  int    m_ippic_count;
};

#endif // _ggsext_mpeg2dec
