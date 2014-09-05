#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <crtdbg.h>
#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "main.h"
#include "device/mpeg2dec.h"

#include <utils/ds_util.h>

#define SET_LOG_FILE(_file_name) {              \
  HANDLE hfile = CreateFile(                \
    _file_name, GENERIC_WRITE, FILE_SHARE_WRITE,    \
    NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  \
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);    \
  _CrtSetReportFile(_CRT_WARN, hfile);          \
}

#if _DEBUG
#define LOG(_fmt, ...) //_CrtDbgReport(_CRT_WARN, NULL, NULL, NULL, _fmt, __VA_ARGS__)
#else
#define LOG(_fmt, ...)
#endif

#define  PICTURE_LOG(_fmt, ...) LOG(_fmt, __VA_ARGS__)
#define  MACROBLOCK_LOG(_fmt, ...) PICTURE_LOG(_fmt, __VA_ARGS__)

enum {
  kBufSize = 0x80000  // 512kb (実測値として最低256kbは必要)
};

enum {
  kPicTypeId_Invalid = 0,
  kPicTypeId_I,
  kPicTypeId_P,
  kPicTypeId_B,
  kPicTypeId_D,
};

enum {
  kChromaFormat_N_A = 0,
  kChromaFormat_4_2_0,
  kChromaFormat_4_2_2,
  kChromaFormat_4_4_4,
};

enum {
  kMBType_Intra      = 0x01,
  kMBType_Pattern      = 0x02,
  kMBType_MotionBackward  = 0x04,
  kMBType_MotionForward  = 0x08,
  kMBType_Quant      = 0x10,
};

enum {
  kPictureStruct_Invalid  = 0,
  kPictureStruct_TopField,
  kPictureStruct_BottomField,
  kPictureStruct_FramePicture,
};

enum {
  kFrameMotionType_Reserved = 0,
  kFrameMotionType_FieldBasedPrediction,
  kFrameMotionType_FrameBasedPrediction,
  kFrameMotionType_DualPrime,
};

enum {
  kFieldMotionType_Reserved = 0,
  kFieldMotionType_FieldBasedPrediction,
  kFieldMotionType_16x8MC,
  kFieldMotionType_DualPrime,
};

enum {
  kMVFormat_Field = 0,
  kMVFormat_Frame,
};

/*--------*/
/* struct */
/*--------*/
typedef struct {
  u32  mask;
  u32  code;
  u8  len;
  u32  value;
} vlc_entry;

typedef struct {
  u16  code;
  u8  len;
  s8  run;
  s16  lv;
} runlength_vlc_entry;

/*----------*/
/* function */
/*----------*/
inline u8 _read_bits(u8* p_base, u8*& p_src, u32& p_src_bit, u32 p_bit_len) {
  u8 result;
  static const u8 mask[8] = { 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

  u8* src0 = (u8*)(p_base + (u32)(p_src - p_base) % kBufSize);
  u8* src1 = (u8*)(p_base + (u32)(p_src - p_base + 1) % kBufSize);

  result  = (*src0 << (p_src_bit)    ) & mask[p_bit_len - 1];
  result |= (*src1 >> (8 - p_src_bit)) & mask[p_bit_len - 1];
  int incbytes = (p_src_bit + p_bit_len) >> 3;
  p_src = (u8*)(p_base + (u32)(p_src - p_base + incbytes) % kBufSize);
  p_src_bit = (p_src_bit + p_bit_len) & 7;
  return result >> (8 - p_bit_len);
}

u32 read_bits(u8* p_base, u8*& p_src, u32& p_src_bit, u32 p_bit_len) {
  u32 out = 0x00000000;
  u32 len = p_bit_len;
  while (1) {
    if (len <= 8) {
      out <<= len;
      out |= _read_bits(p_base, p_src, p_src_bit, len);
      return out;
    } else {
      out <<= 8;
      out |= _read_bits(p_base, p_src, p_src_bit, 8);
      len -= 8;
    }
  }
  return 0;
}

inline u32 fetch_bits(u8* p_base, u8* p_src, u32 p_src_bit, u8 p_bit_len) {
  return read_bits(p_base, p_src, p_src_bit, p_bit_len);
}

u32 vlc_decode(const vlc_entry* p_table, u8* p_base, u8* &p_ptr, u32 &p_src_bit) {
  u32  result = 0x7fffffff;
  u32  vlc = fetch_bits(p_base, p_ptr, p_src_bit, 16);
  for (int i = 0; p_table[i].mask; i++) {
    if ((vlc & p_table[i].mask) != p_table[i].code) {
      continue;
    }
    // 一致したビット長分、実際に進める
    read_bits(p_base, p_ptr, p_src_bit, p_table[i].len);
    result = p_table[i].value;
    break;
  }
  assert(result != 0x7fffffff);
  return result;
}

/*----------*/
/*  method  */
/*----------*/
Mpeg2Decoder::Mpeg2Decoder() :
    m_movie_end(true),
    m_available_render(false),
    m_buf(NULL),
    m_ptr(NULL),
    m_read_ptr(NULL),
    m_horizontal_size_value(0),
    m_vertical_size_value(0),
    m_aspect_ratio_information(0),
    m_frame_rate_code(0),
    m_bit_rate_value(0),
    m_vbv_buffer_size_value(0),
    m_profile_and_level_indication(0),
    m_progressive_sequence(0),
    m_chroma_format(0),
    m_low_delay(0),
    m_video_format(0),
    m_color_primaries(0),
    m_transfer_characteristics(0),
    m_matrix_coefficients(0),
    m_display_horizontal_size(0),
    m_display_vertical_size(0),
    m_scalable_extentions(0),
    m_scalable_mode(0),
    m_layer_id(0),
    m_lower_layer_prediction_horizontal_size(0),
    m_lower_layer_prediction_vertical_size(0),
    m_horizontal_subsampling_factor_m(0),
    m_horizontal_subsampling_factor_n(0),
    m_vertical_subsampling_factor_m(0),
    m_vertical_subsampling_factor_n(0),
    m_mux_to_progressive_sequence(0),
    m_picture_mux_order(0),
    m_picture_mux_factor(0),
    m_intra_dc_precision(0),
    m_picture_struct(0),
    m_top_field_first(0),
    m_frame_predictive_frame_dct(0),
    m_concealment_motion_vectors(0),
    m_q_scale_type(0),
    m_intra_vlc_format(0),
    m_alternate_scan(0),
    m_repeat_first_field(0),
    m_chroma_420_type(0),
    m_progressive_frame(0),
    m_v_axis(0),
    m_field_sequence(0),
    m_sub_carrier(0),
    m_burst_amplitude(0),
    m_sub_carrier_phese(0),
    m_time_code(0),
    m_closed_gop(0),
    m_broken_link(0),
    m_temporal_reference(0),
    m_picture_coding_type(0),
    m_vbv_delay(0),
    m_full_pel_forward_vector(0),
    m_forward_f_code(0),
    m_full_pel_backward_vector(0),
    m_backward_f_code(0),
    m_slice_vertical_position_extension(0),
    m_priority_breakpoint(0),
    m_quantizer_scale_code(0),
    m_intra_slice(0),
    m_macroblock_address_increment(0),
    m_mb_type(0),
    m_frame_motion_type(0),
    m_field_motion_type(0),
    m_dual_prime(0),
    m_mv_format(0),
    m_motion_vector_count(0),
    m_dct_type(0),
    m_dct_dc_differential(0),
    m_rgb_data(NULL),
    m_rgb_pos_x(0),
    m_rgb_pos_y(0),
    m_ippic_count(0) {
  memset(m_intra_quantizer_matrix, 0, sizeof(u8) * 64);
  memset(m_non_intra_quantizer_matrix, 0, sizeof(u8) * 64);
  m_f_code[0][0] = m_f_code[0][1] = m_f_code[1][0] = m_f_code[1][1] = 0;
  
  m_dmvector[0] = m_dmvector[1] = 0;
  memset(m_motion_vertical_field_select, 0, sizeof(u32) * 2 * 2);
  memset(m_motion_code, 0, sizeof(s32) * 2 * 2 * 2);
  memset(m_motion_residual, 0, sizeof(u32) * 2 * 2 * 2);
  memset(m_motion_vector, 0, sizeof(s32) * 2 * 2 * 2);
  memset(m_pattern_code, 0, sizeof(s32) * 12);
  
  m_dct_dc_pred[0] = m_dct_dc_pred[1] = m_dct_dc_pred[2] = 0,
  memset(m_qfs, 0, sizeof(s32) * 64);
  memset(m_qf, 0, sizeof(s32) * 8 * 8);
  memset(m_f, 0, sizeof(s32) * 8 * 8);
  memset(m_yuv, 0, sizeof(s32) * 3 * 16 * 16);

  m_wk_pic[0] = m_wk_pic[1] = m_wk_pic[2] = m_wk_pic[3] = NULL;
}

Mpeg2Decoder::~Mpeg2Decoder() {
  for (int i = 0; i < 4; i++) {
    if (m_wk_pic[i]) free(m_wk_pic[i]);
  }
  if (m_buf) free(m_buf);
}

int Mpeg2Decoder::fill_buffer(const u8* p_src_buf, int p_size) {
  if (m_buf == NULL) return false;
  
  // m_read_ptr == m_ptrの状態は未ロードサイズ == kBufSizeと解釈するため
  // readable_sizeはkBufSize未満である必要がある
  int readable_size;
  if (m_read_ptr >= m_ptr) {
    readable_size = (kBufSize - (int)(m_read_ptr - m_ptr) - 1) & ~0x7ff;
  } else {
    readable_size = ((int)(m_ptr - m_read_ptr) - 1) & ~0x7ff;
  }

  int actual_read_size = p_size < readable_size ? p_size : readable_size;
  if (actual_read_size > 0) {
    if ((int)(m_read_ptr - m_buf) + actual_read_size <= kBufSize) {
      memcpy(m_read_ptr, p_src_buf, actual_read_size);
      m_read_ptr = m_buf + ((int)(m_read_ptr - m_buf) +
                     actual_read_size) % kBufSize;
    } else {
      int first_part_size = kBufSize - (int)(m_read_ptr - m_buf);
      memcpy(m_read_ptr, p_src_buf, first_part_size);
      memcpy(m_buf,
        p_src_buf + first_part_size,
        actual_read_size - first_part_size);
      m_read_ptr = m_buf + ((int)(m_read_ptr - m_buf) +
                     actual_read_size) % kBufSize;
    }
  }
  return actual_read_size;
}

void Mpeg2Decoder::reset() {
  if (m_buf == NULL) {
    m_buf = (u8*)malloc(kBufSize);
  }
  m_ptr = m_buf;
  m_read_ptr = m_buf;

  // 
  // 被参照ピクチャ保持用のバッファの初期化
  for (int i = 0; i < 4; i++) {
    if (m_wk_pic[i]) {
      free(m_wk_pic[i]);
      m_wk_pic[i] = NULL;
    }
  }
  m_ippic_count = 0;

  m_movie_end = false;
}

void Mpeg2Decoder::get_pic(u32* p_dst_buf) {
  //ds_util::DBGOUTA("movie decode proc=%x\n", m_ptr - m_buf);
  m_available_render = true;
  if (m_buf == NULL || m_movie_end) return;
  m_movie_end = decode(p_dst_buf);
}

bool Mpeg2Decoder::decode(u32* p_dst_buf) {
  int processed_picture = 0;

  // 画像展開用のバッファを設定
  m_rgb_data = p_dst_buf;

  // 展開位置を初期化
  m_rgb_pos_x = m_rgb_pos_y = 0;

  bool result;
  while (1) {
    //LOG("baseptr=%x readptr=%x procptr=%x\n", m_buf, m_read_ptr - m_buf, m_ptr - m_buf);
    u32  read_bit = 0;
    u32 data = read_bits(m_buf, m_ptr, read_bit, 32);
    if (data == 0x000001b3) {
      LOG("######### sequence header #########\n");
      sequence_decode(m_ptr, read_bit);
    } else if (data == 0x000001b5) {
      u32 extension_start_code_identifier = read_bits(m_buf, m_ptr, read_bit, 4);
      if (extension_start_code_identifier == 1) {
        LOG("######### sequence extension #########\n");
        sequence_extension_decode(m_ptr, read_bit);
      } else if (extension_start_code_identifier == 2) {
        LOG("######### sequence display extension #########\n");
        sequence_display_extension_decode(m_ptr, read_bit);
      } else if (extension_start_code_identifier == 5) {
        LOG("######### sequence scalable extension #########\n");
        sequence_scalable_extension_decode(m_ptr, read_bit);
      } else if (extension_start_code_identifier == 8) {
        LOG("######### picture coding extension #########\n");
        picture_coding_extension_decode(m_ptr, read_bit);
      } else if (extension_start_code_identifier == 3) {
        LOG("######### quant matrix extension #########\n");
        assert(0);
      } else {
        LOG("######### non supported extension #########\n");
        assert(0);
      }
    } else if (data == 0x000001b2) {
      LOG("######### user data #########\n");
    } else if (data == 0x000001b7) {
      LOG("######### sequence end #########\n");
      result = true;
      break;
    } else if (data == 0x000001b8) {
      LOG("######### gop data #########\n");
      gop_decode(m_ptr, read_bit);
    } else if (data == 0x00000100) {
      // セットアップとして呼ばれた場合と２枚目のピクチャを見つけたら終了
      if (processed_picture++ > 0) {
        m_ptr -= 4;
        result = false;
        break;
      }
      LOG("######### picture data #########\n");
      picture_data_decode(m_ptr, read_bit);

      // 被参照ピクチャをローテート保持
      if (m_picture_coding_type == kPicTypeId_I
      ||  m_picture_coding_type == kPicTypeId_P) {
        s32* tempbuf = m_wk_pic[3];
        m_wk_pic[3] = m_wk_pic[2];
        m_wk_pic[2] = m_wk_pic[1];
        m_wk_pic[1] = m_wk_pic[0];
        m_wk_pic[0] = tempbuf;
        m_ippic_count++;
      }
    } else if (data >= 0x00000101 && data <= 0x000001af) {
      PICTURE_LOG("######### slice data #########\n");
      slice_decode(m_ptr, read_bit);
      do {
        PICTURE_LOG("######### macroblock data (%d, %d) #########\n", m_rgb_pos_x, m_rgb_pos_y);
        macro_block_decode(m_ptr, read_bit);
        macro_block_render();

        m_rgb_pos_x += 16;
        if (m_rgb_pos_x >= m_horizontal_size_value) {
          m_rgb_pos_x = 0;
          m_rgb_pos_y += 16;
        }
      } while (fetch_bits(m_buf, m_ptr, read_bit, 23) != 0 || m_macroblock_address_increment > 0);
    } else {
      // 32bitのヘッダコード分を戻しつつ1byte進める
      m_ptr -= 3;
    }
  }

  return result;
}

void Mpeg2Decoder::sequence_decode(u8* &p_ptr, u32 &p_src_bit) {
  static const int kDefaultDequantizeTable[2][64] = {
    8,  16,  19,  22,  26,  27,  29,  34,
    16,  16,  22,  24,  27,  29,  34,  37,
    19,  22,  26,  27,  29,  34,  34,  38,
    22,  22,  26,  27,  29,  34,  37,  40,
    22,  26,  27,  29,  32,  35,  40,  48,
    26,  27,  29,  32,  35,  40,  48,  58,
    26,  27,  29,  34,  38,  46,  56,  69,
    27,  29,  35,  38,  46,  56,  69,  83,

    16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,
  };
  const int kZigzagTable[8][8] = {
    /* Zig-Zag scan pattern */
    0,  1,  8,  16,  9,  2,  3,  10,
    17,  24,  32,  25,  18,  11,  4,  5,
    12,  19,  26,  33,  40,  48,  41,  34,
    27,  20,  13,  6,  7,  14,  21,  28,
    35,  42,  49,  56,  57,  50,  43,  36,
    29,  22,  15,  23,  30,  37,  44,  51,
    58,  59,  52,  45,  38,  31,  39,  46,
    53,  60,  61,  54,  47,  55,  62,  63
  };
  m_horizontal_size_value = read_bits(m_buf, p_ptr, p_src_bit, 12);
  LOG("horizontal_size_value=%d\n", m_horizontal_size_value);

  m_vertical_size_value = read_bits(m_buf, p_ptr, p_src_bit, 12);
  LOG("vertical_size_value=%d\n", m_vertical_size_value);

  const float kAspectRatioTbl[] = {
    0.0000f, 1.0000f, 0.6735f, 0.7031f, 0.7615f, 0.8055f, 0.8437f, 0.8935f,
    0.9157f, 0.9815f, 1.0255f, 1.0695f, 1.0950f, 1.1575f, 1.2015f, 0.0000f,
  };
  m_aspect_ratio_information = read_bits(m_buf, p_ptr, p_src_bit, 4);
  LOG("aspect_ratio_information=%.3f\n", kAspectRatioTbl[m_aspect_ratio_information]);
  
  const float kPictureRateTbl[] = {
    0.0000f, 23.976f, 24.000f, 25.000f, 29.970f, 30.000f, 50.000f, 59.940f,
    60.000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  };
  m_frame_rate_code = read_bits(m_buf, p_ptr, p_src_bit, 4);
  LOG("frame_rate=%.3f\n", kPictureRateTbl[m_frame_rate_code]);

  m_bit_rate_value = read_bits(m_buf, p_ptr, p_src_bit, 18);
  LOG("bit_rate_value=%f(mbps)\n", (float)m_bit_rate_value * 0.0004f);

  u32 marker_bit = read_bits(m_buf, p_ptr, p_src_bit, 1);
  assert(marker_bit == 1);

  m_vbv_buffer_size_value = read_bits(m_buf, p_ptr, p_src_bit, 10);
  LOG("video_buf_verifier=%d\n", m_vbv_buffer_size_value * 16 * 1024);

  u32  constrained_parameter_flag = read_bits(m_buf, p_ptr, p_src_bit, 1);
  assert(constrained_parameter_flag == 0);

  u32 load_intra_quantizer_matrix = read_bits(m_buf, p_ptr, p_src_bit, 1);

  for (int i = 0; i < 64; i++) {
    //m_intra_quantizer_matrix[kZigzagTable[i >> 3][i & 7]] = load_intra_quantizer_matrix ? (u8)read_bits(m_buf, p_ptr, p_src_bit, 8) : kDefaultDequantizeTable[0][i];
    m_intra_quantizer_matrix[i] = load_intra_quantizer_matrix ? (u8)read_bits(m_buf, p_ptr, p_src_bit, 8) : kDefaultDequantizeTable[0][i];
  }
#if defined _DEBUG
  LOG("intra_quantizer_matrix[]={\n");
  for (int i = 0; i < 8; i++) {
    LOG("  %3d %3d %3d %3d %3d %3d %3d %3d\n",
      m_intra_quantizer_matrix[i * 8 + 0],
      m_intra_quantizer_matrix[i * 8 + 1],
      m_intra_quantizer_matrix[i * 8 + 2],
      m_intra_quantizer_matrix[i * 8 + 3],
      m_intra_quantizer_matrix[i * 8 + 4],
      m_intra_quantizer_matrix[i * 8 + 5],
      m_intra_quantizer_matrix[i * 8 + 6],
      m_intra_quantizer_matrix[i * 8 + 7]
    );
  }
  LOG("}\n");
#endif // _DEBUG

  u32 load_non_intra_quantizer_matrix = read_bits(m_buf, p_ptr, p_src_bit, 1);
  for (int i = 0; i < 64; i++) {
    m_non_intra_quantizer_matrix[kZigzagTable[i >> 3][i & 7]] = load_non_intra_quantizer_matrix ? (u8)read_bits(m_buf, p_ptr, p_src_bit, 8) : kDefaultDequantizeTable[1][i];
  }
#if defined _DEBUG
  LOG("non_intra_quantizer_matrix[]={\n");
  for (int i = 0; i < 8; i++) {
    LOG("  %3d %3d %3d %3d %3d %3d %3d %3d\n",
      m_non_intra_quantizer_matrix[i * 8 + 0],
      m_non_intra_quantizer_matrix[i * 8 + 1],
      m_non_intra_quantizer_matrix[i * 8 + 2],
      m_non_intra_quantizer_matrix[i * 8 + 3],
      m_non_intra_quantizer_matrix[i * 8 + 4],
      m_non_intra_quantizer_matrix[i * 8 + 5],
      m_non_intra_quantizer_matrix[i * 8 + 6],
      m_non_intra_quantizer_matrix[i * 8 + 7]
    );
  }
  LOG("}\n");
#endif // _DEBUG
}

void Mpeg2Decoder::sequence_extension_decode(u8* &p_ptr, u32 &p_src_bit) {
  m_profile_and_level_indication = read_bits(m_buf, p_ptr, p_src_bit, 8);
  LOG("  profile_and_level_indication=%d\n", m_profile_and_level_indication);

  m_progressive_sequence = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("  progressive_sequence=%d\n", m_progressive_sequence);

  m_chroma_format = read_bits(m_buf, p_ptr, p_src_bit, 2);
  const s8 chroma_format_label[4][16] = { "N/A", "4:2:0", "4:2:2", "4:4:4" };
  LOG("  chroma_format=%s\n", chroma_format_label[m_chroma_format]);

  u32 horizontal_size_extension = read_bits(m_buf, p_ptr, p_src_bit, 2);
  LOG("  horizontal_size_extension=%d\n", horizontal_size_extension);
  m_horizontal_size_value |= horizontal_size_extension << 12;

  u32 vertical_size_extension = read_bits(m_buf, p_ptr, p_src_bit, 2);
  LOG("  vertical_size_extension=%d\n", vertical_size_extension);
  m_vertical_size_value |= vertical_size_extension << 12;

  u32 bit_rate_extension = read_bits(m_buf, p_ptr, p_src_bit, 12);
  LOG("  bit_rate_extension=%d\n", bit_rate_extension);
  m_bit_rate_value |= bit_rate_extension << 18;

  u32 vbv_buffer_size_extension = read_bits(m_buf, p_ptr, p_src_bit, 8);
  LOG("  vbv_buffer_size_extension=%d\n", vbv_buffer_size_extension);
  m_vbv_buffer_size_value |= vbv_buffer_size_extension << 10;

  m_low_delay = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("  low_delay=%d\n", m_low_delay);

  u32 frame_rate_extension_n = read_bits(m_buf, p_ptr, p_src_bit, 2);
  LOG("  frame_rate_extension_n=%d\n", frame_rate_extension_n);

  u32 frame_rate_extension_d = read_bits(m_buf, p_ptr, p_src_bit, 2);
  LOG("  frame_rate_extension_d=%d\n", frame_rate_extension_d);
  m_frame_rate_code = m_frame_rate_code * (frame_rate_extension_n + 1) / (frame_rate_extension_d + 1);
}

void Mpeg2Decoder::sequence_display_extension_decode(u8* &p_ptr, u32 &p_src_bit) {
  m_video_format = read_bits(m_buf, p_ptr, p_src_bit, 3);
  const s8 video_format_label[8][16] = { "component", "PAL", "NTSC", "SECAM", "MAC", "Unspecified", "reserved", "reverved" };
  LOG("  video_format=%s\n", video_format_label[m_video_format]);

  u32 color_description = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("  color_description=%d\n", color_description);
  
  if (color_description) {
    m_color_primaries = read_bits(m_buf, p_ptr, p_src_bit, 8);
    LOG("  color_primaries=%d\n", m_color_primaries);

    m_transfer_characteristics = read_bits(m_buf, p_ptr, p_src_bit, 8);
    LOG("  transfer_characteristics=%d\n", m_transfer_characteristics);

    m_matrix_coefficients = read_bits(m_buf, p_ptr, p_src_bit, 8);
    LOG("  matrix_coefficients=%d\n", m_matrix_coefficients);
  }
  m_display_horizontal_size = read_bits(m_buf, p_ptr, p_src_bit, 14);
  LOG("  display_horizontal_size=%d\n", m_display_horizontal_size);

  u32 marker_bit = read_bits(m_buf, p_ptr, p_src_bit, 1);
  assert(marker_bit == 1);

  m_display_vertical_size = read_bits(m_buf, p_ptr, p_src_bit, 14);
  LOG("  display_vertical_size=%d\n", m_display_vertical_size);
}

void Mpeg2Decoder::sequence_scalable_extension_decode(u8* &p_ptr, u32 &p_src_bit) {
  m_scalable_extentions = true;

  m_scalable_mode = read_bits(m_buf, p_ptr, p_src_bit, 2);
  LOG("  scalable_mode=%d\n", m_scalable_mode);

  m_layer_id = read_bits(m_buf, p_ptr, p_src_bit, 4);
  LOG("  layer_id=%d\n", m_layer_id);

  m_lower_layer_prediction_horizontal_size = read_bits(m_buf, p_ptr, p_src_bit, 14);
  LOG("  lower_layer_prediction_horizontal_size=%d\n", m_lower_layer_prediction_horizontal_size);

  m_lower_layer_prediction_vertical_size = read_bits(m_buf, p_ptr, p_src_bit, 14);
  LOG("  lower_layer_prediction_vertical_size=%d\n", m_lower_layer_prediction_vertical_size);

  m_horizontal_subsampling_factor_m = read_bits(m_buf, p_ptr, p_src_bit, 5);
  LOG("  horizontal_subsampling_factor_m=%d\n", m_horizontal_subsampling_factor_m);

  m_horizontal_subsampling_factor_n = read_bits(m_buf, p_ptr, p_src_bit, 5);
  LOG("  horizontal_subsampling_factor_n=%d\n", m_horizontal_subsampling_factor_n);

  m_vertical_subsampling_factor_m = read_bits(m_buf, p_ptr, p_src_bit, 5);
  LOG("  vertical_subsampling_factor_m=%d\n", m_vertical_subsampling_factor_m);

  m_vertical_subsampling_factor_n = read_bits(m_buf, p_ptr, p_src_bit, 5);
  LOG("  vertical_subsampling_factor_n=%d\n", m_vertical_subsampling_factor_n);

  u32  picture_mux_enable = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("  picture_mux_enable=%d\n", picture_mux_enable);
  
  if (picture_mux_enable) {
    m_mux_to_progressive_sequence = read_bits(m_buf, p_ptr, p_src_bit, 1);
    LOG("  mux_to_progressive_sequence=%d\n", m_mux_to_progressive_sequence);

    m_picture_mux_order = read_bits(m_buf, p_ptr, p_src_bit, 3);
    LOG("  picture_mux_order=%d\n", m_picture_mux_order);

    m_picture_mux_factor = read_bits(m_buf, p_ptr, p_src_bit, 3);
    LOG("  picture_mux_factor=%d\n", m_picture_mux_factor);
  }
}

void Mpeg2Decoder::picture_coding_extension_decode(u8* &p_ptr, u32 &p_src_bit) {
  m_f_code[0][0] = read_bits(m_buf, p_ptr, p_src_bit, 4);
  LOG("forward_horizontal_f_code=%d\n", m_f_code[0][0]);

  m_f_code[0][1] = read_bits(m_buf, p_ptr, p_src_bit, 4);
  LOG("forward_vertical_f_code=%d\n", m_f_code[0][1]);

  m_f_code[1][0] = read_bits(m_buf, p_ptr, p_src_bit, 4);
  LOG("backward_horizontal_f_code=%d\n", m_f_code[1][0]);

  m_f_code[1][1] = read_bits(m_buf, p_ptr, p_src_bit, 4);
  LOG("backward_vertical_f_code=%d\n", m_f_code[1][1]);

  m_intra_dc_precision = read_bits(m_buf, p_ptr, p_src_bit, 2);
  LOG("intra_dc_precision=%dbits\n", m_intra_dc_precision + 8);

  m_picture_struct = read_bits(m_buf, p_ptr, p_src_bit, 2);
  const s8 picture_struct_label[4][16] = { "reserved", "top_field", "bottom_field", "frame_picture" };
  LOG("picture_struct=%s\n", picture_struct_label[m_picture_struct]);

  m_top_field_first = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("top_field_first=%d\n", m_top_field_first);

  m_frame_predictive_frame_dct = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("frame_predictive_frame_dct=%d\n", m_frame_predictive_frame_dct);

  m_concealment_motion_vectors = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("concealment_motion_vectors=%d\n", m_concealment_motion_vectors);

  m_q_scale_type = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("q_scale_type=%d\n", m_q_scale_type);

  m_intra_vlc_format = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("intra_vlc_format=%d\n", m_intra_vlc_format);

  m_alternate_scan = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("alternate_scan=%d\n", m_alternate_scan);

  m_repeat_first_field = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("repeat_first_field=%d\n", m_repeat_first_field);

  m_chroma_420_type = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("chroma_420_type=%d\n", m_chroma_420_type);

  m_progressive_frame = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("progressive_frame=%d\n", m_progressive_frame);

  u32 composite_display_flag = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("composite_display_flag=%d\n", composite_display_flag);

  if (composite_display_flag) {
    m_v_axis = read_bits(m_buf, p_ptr, p_src_bit, 1);
    LOG("v_axis=%d\n", m_v_axis);

    m_field_sequence = read_bits(m_buf, p_ptr, p_src_bit, 3);
    LOG("field_sequence=%d\n", m_field_sequence);

    m_sub_carrier = read_bits(m_buf, p_ptr, p_src_bit, 1);
    LOG("sub_carrier=%d\n", m_sub_carrier);

    m_burst_amplitude = read_bits(m_buf, p_ptr, p_src_bit, 7);
    LOG("burst_amplitude=%d\n", m_burst_amplitude);

    m_sub_carrier_phese = read_bits(m_buf, p_ptr, p_src_bit, 8);
    LOG("sub_carrier_phese=%d\n", m_sub_carrier_phese);
  }
}

void Mpeg2Decoder::gop_decode(u8* &p_ptr, u32 &p_src_bit) {
  m_time_code = read_bits(m_buf, p_ptr, p_src_bit, 25);
  LOG("  time_code=%d:%d:%d:%d\n", (m_time_code >> 19) & 0x1f, (m_time_code >> 13) & 0x3f, (m_time_code >> 6) & 0x3f, m_time_code & 0x3f);
  assert(m_time_code & 0x1000);  // MarkerBit
  
  m_closed_gop = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("  closed_gop=%d\n", m_closed_gop);

  m_broken_link = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("  broken_link=%d\n", m_broken_link);
}

void Mpeg2Decoder::picture_data_decode(u8* &p_ptr, u32 &p_src_bit) {
  m_temporal_reference = read_bits(m_buf, p_ptr, p_src_bit, 10);
  LOG("  temporal_reference=%d\n", m_temporal_reference);
  
  m_picture_coding_type = read_bits(m_buf, p_ptr, p_src_bit, 3);
  const s8 picture_coding_type_label[8][8] = { "n/a", "I", "P", "B", "D", "n/a", "n/a", "n/a" };
  LOG("  picture_coding_type=%s\n", picture_coding_type_label[m_picture_coding_type]);
  
  m_vbv_delay = read_bits(m_buf, p_ptr, p_src_bit, 16);
  LOG("  vbv_delay=%d\n", m_vbv_delay);

  if (m_picture_coding_type == kPicTypeId_B
  ||  m_picture_coding_type == kPicTypeId_D) {
    m_full_pel_forward_vector = read_bits(m_buf, p_ptr, p_src_bit, 1);
    LOG("  full_pel_forward_vector=%d\n", m_full_pel_forward_vector);

    m_forward_f_code = read_bits(m_buf, p_ptr, p_src_bit, 3);
    LOG("  forward_f_code=%d\n", m_forward_f_code);
  }
  if (m_picture_coding_type == kPicTypeId_D) {
    m_full_pel_backward_vector = read_bits(m_buf, p_ptr, p_src_bit, 1);
    LOG("  full_pel_backward_vector=%d\n", m_full_pel_backward_vector);

    m_backward_f_code = read_bits(m_buf, p_ptr, p_src_bit, 3);
    LOG("  backward_f_code=%d\n", m_backward_f_code);
  }

  u32 extra_bit_picture = read_bits(m_buf, p_ptr, p_src_bit, 1);
  LOG("  extra_bit_picture=%d\n", extra_bit_picture);
  while (extra_bit_picture) {
    u32 extra_information_picture = read_bits(m_buf, p_ptr, p_src_bit, 8);
    LOG("  extra_information_picture=%d\n", extra_information_picture);

    extra_bit_picture = read_bits(m_buf, p_ptr, p_src_bit, 1);
    LOG("  extra_bit_picture=%d\n", extra_bit_picture);
  }

  // 被参照ピクチャ保持用のバッファを作成
  int macro_block_count = (m_horizontal_size_value >> 4) * (m_vertical_size_value >> 4);
  const int kPixelPerBlock = 8 * 8;
  const int kBlockPerMacroBlock = 4;
  const int kChannelCount = 3;
  // 実際のサイズ
  const int kWorkMemSize = macro_block_count * kPixelPerBlock * kBlockPerMacroBlock * kChannelCount;
  // 演算簡略化の為の余白(上下左右の1ピクセル分)
  const int kMarginMemSize = (m_horizontal_size_value + m_vertical_size_value + 2) * kChannelCount * 2;
  for (int i = 0; i < 4; i++) {
    if (m_wk_pic[i] == NULL) {
      m_wk_pic[i] = (s32*)malloc(sizeof(s32) * (kWorkMemSize + kMarginMemSize));

      for (int j = 0; j < kWorkMemSize + kMarginMemSize; j += 3) {
        m_wk_pic[i][j + 0] = 16;
        m_wk_pic[i][j + 1] = 128;
        m_wk_pic[i][j + 2] = 128;
      }
    }
  }
}

void Mpeg2Decoder::slice_decode(u8* &p_ptr, u32 &p_src_bit) {

  //PICTURE_LOG("reset dc precision due to slice section beginning\n");
  m_dct_dc_pred[0] = 128 << m_intra_dc_precision;
  m_dct_dc_pred[1] = m_dct_dc_pred[0];
  m_dct_dc_pred[2] = m_dct_dc_pred[0];

  //PICTURE_LOG("reset motion vector due to slice section beginning\n");
  for (int i = 0; i < 8; i++) {
    m_motion_vector[i >> 2 & 1][i >> 1 & 1][i & 1] = 0;
  }

  if (m_vertical_size_value > 2800) {
    m_slice_vertical_position_extension = read_bits(m_buf, p_ptr, p_src_bit, 3);
    PICTURE_LOG("  slice_vertical_position_extension=%d\n", m_slice_vertical_position_extension);
  }

  if (m_scalable_extentions) {
    m_priority_breakpoint = read_bits(m_buf, p_ptr, p_src_bit, 7);
    PICTURE_LOG("  priority_breakpoint=%d\n", m_priority_breakpoint);
  }

  m_quantizer_scale_code = read_bits(m_buf, p_ptr, p_src_bit, 5);
  //PICTURE_LOG("  quantizer_scale_code=%d\n", m_quantizer_scale_code);
  
  u32 intra_slice_flag = read_bits(m_buf, p_ptr, p_src_bit, 1);
  //PICTURE_LOG("  intra_slice_flag=%d\n", intra_slice_flag);

  if (intra_slice_flag) {
    m_intra_slice = read_bits(m_buf, p_ptr, p_src_bit, 1);
    PICTURE_LOG("  intra_slice=%d\n", m_intra_slice);

    u32 reserved_bits = read_bits(m_buf, p_ptr, p_src_bit, 7);
    PICTURE_LOG("  reserved_bits=%d\n", reserved_bits);

    u32 extra_bit_slice = read_bits(m_buf, p_ptr, p_src_bit, 1);
    PICTURE_LOG("  extra_bit_slice=%d\n", extra_bit_slice);

    while (extra_bit_slice) {
      u32 extra_information_slice = read_bits(m_buf, p_ptr, p_src_bit, 8);
      PICTURE_LOG("  extra_information_slice=%d\n", extra_information_slice);

      extra_bit_slice = read_bits(m_buf, p_ptr, p_src_bit, 1);
      PICTURE_LOG("  extra_bit_slice=%d\n", extra_bit_slice);
    }
  }
}

void Mpeg2Decoder::macro_block_decode(u8* &p_ptr, u32 &p_src_bit) {

  if (m_macroblock_address_increment == 0) {
    static const vlc_entry kMbaiVLCTbl[] = { // mpeg2ではmacroblock_stuffingは使用されない
      0x400, 0x400,  1,  1,  0x700, 0x300,  3,  2,  0x700, 0x200,  3,  3,  0x780, 0x180,  4,  4,
      0x780, 0x100,  4,  5,  0x7c0, 0x0c0,  5,  6,  0x7c0, 0x080,  5,  7,  0x7f0, 0x070,  7,  8,
      0x7f0, 0x060,  7,  9,  0x7f8, 0x058,  8, 10,  0x7f8, 0x050,  8, 11,  0x7f8, 0x048,  8, 12,
      0x7f8, 0x040,  8, 13,  0x7f8, 0x038,  8, 14,  0x7f8, 0x030,  8, 15,  0x7fe, 0x02e, 10, 16,
      0x7fe, 0x02c, 10, 17,  0x7fe, 0x02a, 10, 18,  0x7fe, 0x028, 10, 19,  0x7fe, 0x026, 10, 20,
      0x7fe, 0x024, 10, 21,  0x7ff, 0x023, 11, 22,  0x7ff, 0x022, 11, 23,  0x7ff, 0x021, 11, 24,
      0x7ff, 0x020, 11, 25,  0x7ff, 0x01f, 11, 26,  0x7ff, 0x01e, 11, 27,  0x7ff, 0x01d, 11, 28,
      0x7ff, 0x01c, 11, 29,  0x7ff, 0x01b, 11, 30,  0x7ff, 0x01a, 11, 31,  0x7ff, 0x019, 11, 32,
      0x7ff, 0x018, 11, 33,  0x7ff, 0x008, 11, -1,  0x000, 0x000,  0,  0,
    };
    m_macroblock_address_increment = 0;
    u32  mbai_vlc = fetch_bits(m_buf, p_ptr, p_src_bit, 11);
    for (int i = 0; kMbaiVLCTbl[i].mask; i++) {
      if ((mbai_vlc & kMbaiVLCTbl[i].mask) != kMbaiVLCTbl[i].code) {
        continue;
      }
      // 一致したビット長分、実際に進める
      read_bits(m_buf, p_ptr, p_src_bit, kMbaiVLCTbl[i].len);

      if (kMbaiVLCTbl[i].value == -1) { // macroblock_escape
        m_macroblock_address_increment += 33;
        mbai_vlc = fetch_bits(m_buf, p_ptr, p_src_bit, 11);
        i = -1;
      } else {
        m_macroblock_address_increment += kMbaiVLCTbl[i].value;
        break;
      }
    }
    //PICTURE_LOG("  macroblock_address_increment=%d\n", m_macroblock_address_increment);

    if (m_macroblock_address_increment > 1) {
      // スキップされる場合、以下の情報を1度だけクリア
      if (m_picture_coding_type == kPicTypeId_P) {
        m_mb_type = m_mb_type & ~kMBType_Pattern | kMBType_MotionForward;

        MACROBLOCK_LOG("reset motion vector due to macro block skipping\n");
        for (int i = 0; i < 8; i++) {
          m_motion_vector[i >> 2 & 1][i >> 1 & 1][i & 1] = 0;
        }
      } else if (m_picture_coding_type == kPicTypeId_B) {
        // 直前の予測方向＆動きベクトルを使用。即ちリセットを行わない。
        m_mb_type = m_mb_type & ~kMBType_Pattern;
      } else {
        assert(0);
      }
      MACROBLOCK_LOG("reset dc precision due to macro block skipping\n");
      m_dct_dc_pred[0] = 128 << m_intra_dc_precision;
      m_dct_dc_pred[1] = m_dct_dc_pred[0];
      m_dct_dc_pred[2] = m_dct_dc_pred[0];

      
      for (int i = 0; i < 12; i++) {
        clear_yuvdata(i);
      }
    }
  }
  m_macroblock_address_increment--;
  if (m_macroblock_address_increment > 0) {
    return;
  }

  static const vlc_entry kMbTypeVLCTab[4][12] = {
    // I-pic
    0x8000, 0x8000, 1, 0x01,  0xc000, 0x4000, 2, 0x11,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,
    0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,
    0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,
    // P-pic
    0x8000, 0x8000, 1, 0x0a,  0xc000, 0x4000, 2, 0x02,  0xe000, 0x2000, 3, 0x08,  0xf800, 0x1800, 5, 0x01,
    0xf800, 0x1000, 5, 0x1a,  0xf800, 0x0800, 5, 0x12,  0xfc00, 0x0400, 6, 0x11,  0x0000, 0x0000, 0, 0x00,
    0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,
    // B-pic
    0xc000, 0x8000, 2, 0x0c,  0xc000, 0xc000, 2, 0x0e,  0xe000, 0x4000, 3, 0x04,  0xe000, 0x6000, 3, 0x06,
    0xf000, 0x2000, 4, 0x08,  0xf000, 0x3000, 4, 0x0a,  0xf800, 0x1800, 5, 0x01,  0xf800, 0x1000, 5, 0x1e,
    0xfc00, 0x0c00, 6, 0x1a,  0xfc00, 0x0800, 6, 0x16,  0xfc00, 0x0400, 6, 0x11,  0x0000, 0x0000, 0, 0x00,
    // D-pic
    0x8000, 0x8000, 1, 0x11,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,
    0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,
    0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,  0x0000, 0x0000, 0, 0x00,
  };
  m_mb_type = vlc_decode(kMbTypeVLCTab[m_picture_coding_type - 1], m_buf, p_ptr, p_src_bit);
  PICTURE_LOG("  mb_type=%02x\n", m_mb_type);

  //MACROBLOCK_LOG("  macroblock_type\n");
  //MACROBLOCK_LOG("    intra=%d\n", !!(m_mb_type & kMBType_Intra));
  //MACROBLOCK_LOG("    pattern=%d\n", !!(m_mb_type & kMBType_Pattern));
  //MACROBLOCK_LOG("    motion_backward=%d\n", !!(m_mb_type & kMBType_MotionBackward));
  //MACROBLOCK_LOG("    motion_forward=%d\n", !!(m_mb_type & kMBType_MotionForward));
  //MACROBLOCK_LOG("    quant=%d\n", !!(m_mb_type & kMBType_Quant));

  if (m_scalable_extentions) {
    u32 spatial_temp_weight_code = read_bits(m_buf, p_ptr, p_src_bit, 2);
    MACROBLOCK_LOG("  spatial_temporal_weight_code=%d\n", spatial_temp_weight_code);
  }

  m_frame_motion_type = kFrameMotionType_Reserved;
  m_field_motion_type = kFieldMotionType_Reserved;

  if (m_mb_type & (kMBType_MotionForward | kMBType_MotionBackward)) {
    if (m_picture_struct == kPictureStruct_FramePicture) {
      if (m_frame_predictive_frame_dct == 0) {
        m_frame_motion_type = read_bits(m_buf, p_ptr, p_src_bit, 2);
        const s8 frame_type_label[4][32] = { "reserved", "field-based prediction", "frame-based prediction", "dual-prime" };
        MACROBLOCK_LOG("  frame_motion_type=%s\n", frame_type_label[m_frame_motion_type]);
        assert(m_frame_motion_type != kFrameMotionType_Reserved);
        m_dual_prime = m_frame_motion_type == kFrameMotionType_DualPrime;
        m_mv_format = m_frame_motion_type == kFrameMotionType_FrameBasedPrediction ? kMVFormat_Frame : kMVFormat_Field;
        m_motion_vector_count = m_frame_motion_type == kFrameMotionType_FieldBasedPrediction ? 2 : 1;
      } else {
        m_frame_motion_type = kFrameMotionType_FrameBasedPrediction;
        m_motion_vector_count = 1;
        m_mv_format = kMVFormat_Frame;
        m_dual_prime = false;
      }
    } else {
      m_field_motion_type = read_bits(m_buf, p_ptr, p_src_bit, 2);
      const s8 field_type_label[4][32] = { "reserved", "field-based prediction", "16x8 MC", "dual-prime" };
      MACROBLOCK_LOG("  field_motion_type=%d\n", field_type_label[m_field_motion_type]);
      assert(m_field_motion_type != kFieldMotionType_Reserved);

      m_dual_prime = m_field_motion_type == kFieldMotionType_DualPrime;
      m_mv_format = kMVFormat_Field;
      m_motion_vector_count = m_field_motion_type == kFieldMotionType_16x8MC ? 2 : 1;
    }
  }

  if (m_picture_struct == kPictureStruct_FramePicture
  &&  m_frame_predictive_frame_dct == 0
  &&  (m_mb_type & (kMBType_Intra | kMBType_Pattern))) {
    m_dct_type = read_bits(m_buf, p_ptr, p_src_bit, 1);
    MACROBLOCK_LOG("  dct_type=%d\n", m_dct_type);
  } else {
    // dct_typeを保持していない
    m_dct_type = -1;
  }

  if (m_mb_type & kMBType_Quant) {
    m_quantizer_scale_code = read_bits(m_buf, p_ptr, p_src_bit, 5);
    MACROBLOCK_LOG("  quantizer_scale_code(macroblock)=%d\n", m_quantizer_scale_code);
  }

  // m_motion_codeの初期化
  for (int i = 0; i < 8; i++) {
    m_motion_code[i >> 2 & 1][i >> 1 & 1][i & 1] = 0;
  }

  // 無条件で初期化してよい？
  m_motion_vertical_field_select[0][0] = 0;
  m_motion_vertical_field_select[0][1] = 0;
  m_motion_vertical_field_select[1][0] = 1;
  m_motion_vertical_field_select[1][1] = 1;

  if ((m_mb_type & kMBType_MotionForward)
  ||  (m_mb_type & kMBType_Intra) && m_concealment_motion_vectors) {
    motion_vectors_decode(p_ptr, p_src_bit, 0);
  }

  if (m_mb_type & kMBType_MotionBackward) {
    motion_vectors_decode(p_ptr, p_src_bit, 1);
  }

  if (m_mb_type & kMBType_Intra) {
    if (m_concealment_motion_vectors) {
      u32 marker_bit = read_bits(m_buf, p_ptr, p_src_bit, 1);
      assert(marker_bit == 1);
    }

    MACROBLOCK_LOG("reset motion vector due to intra block\n");
    for (int i = 0; i < 8; i++) {
      m_motion_vector[i >> 2 & 1][i >> 1 & 1][i & 1] = 0;
    }
  } else {
    const bool interlace = m_picture_struct == kPictureStruct_FramePicture && m_mv_format == kMVFormat_Field;
    // モーション差分の取得
    if (m_picture_struct == kPictureStruct_FramePicture) {
      if (m_frame_motion_type == kFrameMotionType_FrameBasedPrediction) {
        motion_vector_calc(false);
      } else if (m_frame_motion_type == kFrameMotionType_FieldBasedPrediction) {
        motion_vector_calc(true);
      } else if (m_frame_motion_type == kFrameMotionType_DualPrime) {
        assert(0);
      } else {
        for (int i = 0; i < 8; i++) {
          m_motion_vector[i >> 2 & 1][i >> 1 & 1][i & 1] = 0;
        }
      }
    } else {
      if (m_field_motion_type == kFieldMotionType_FieldBasedPrediction) {
        assert(0);
      } else if (m_field_motion_type == kFieldMotionType_16x8MC) {
        assert(0);
      } else if (m_field_motion_type == kFieldMotionType_DualPrime) {
        assert(0);
      } else {
        for (int i = 0; i < 8; i++) {
          m_motion_vector[i >> 2 & 1][i >> 1 & 1][i & 1] = 0;
        }
      }
    }

    // DC予測値をリセット
    MACROBLOCK_LOG("reset dc precision due to non intra block\n");
    m_dct_dc_pred[0] = 128 << m_intra_dc_precision;
    m_dct_dc_pred[1] = m_dct_dc_pred[0];
    m_dct_dc_pred[2] = m_dct_dc_pred[0];
  }

  coded_block_pattern_decode(p_ptr, p_src_bit);

  for (int i = 0; i < 12; i++) {
    if (m_pattern_code[i]) {
      bool result = block_decode(p_ptr, p_src_bit, i);

      inverse_zigzag();
      dequantize(i);
      decode_dct(i);
      if (result) break;
    } else {
      clear_yuvdata(i);
    }
  }
}

void Mpeg2Decoder::macro_block_render() {

  int stride = (m_mv_format == kMVFormat_Frame ? 1 : 2);
  const int width_bytes  = (m_horizontal_size_value + 2) * 3;

  // 参照ピクチャ／フィールドごとに参照ピクセルのオフセットを算出しておく
  // トップフィールド／ボトムフィールドの参照先はm_motion_vertical_field_selectによって切り替える
  int oddx[2][2], oddy[2][2];
  int src_offset[2][2];
  if (m_mb_type & (kMBType_MotionForward | kMBType_MotionBackward | kMBType_Pattern)) {
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        u32 mv;    // どちらのベクトルを使用するか？フレームフォーマットなら常にmv1
        if (m_mv_format == kMVFormat_Frame) {
          mv = 0;
        } else {
          mv = i;
        }

        int pos2x = m_rgb_pos_x * 2;
        int pos2y = m_rgb_pos_y * 2;
        if (m_mb_type & (kMBType_MotionForward | kMBType_MotionBackward)) {
          pos2x += m_motion_vector[mv][j][0];
          pos2y += m_motion_vector[mv][j][1];
        }

        if (pos2x < 0)        { pos2x = 0; }
        if (pos2x > (640 - 16) * 2)  { pos2x = (640 - 16) * 2; }
        if (pos2y < 0)        { pos2y = 0; }
        if (pos2y > (480 - 16) * 2)  { pos2y = (480 - 16) * 2; }

        int posx  = pos2x / 2;
        int posy  = pos2y / 2;
        
        // フィールドの場合
        if (m_mv_format == kMVFormat_Field) {
          posy = (posy & 0xfffffffe) + m_motion_vertical_field_select[mv][j];
        } else {
          posy += i;
        }

        src_offset[i][j] = (posy + 1) * width_bytes + (posx + 1) * 3;
        oddx[i][j] = (pos2x & 1) * (pos2x & 0x80000000 ? -1 : 1) * 3;
        if (m_mv_format == kMVFormat_Field) {
          oddy[i][j] = (pos2y / 2 & 1) * (pos2y & 0x80000000 ? -1 : 1) * width_bytes * stride;
        } else {
          oddy[i][j] = (pos2y     & 1) * (pos2y & 0x80000000 ? -1 : 1) * width_bytes * stride;
        }
        MACROBLOCK_LOG("mv%d(%s) pos=(%d,%d) odd=(%d,%d)\n", i, j == 0 ? "fw" : "bk", posx, posy, oddx[i][j] / 3, oddy[i][j] / (width_bytes * stride));
      }
    }
  }

  int dst_offset  = (m_rgb_pos_y + 1) * width_bytes + (m_rgb_pos_x + 1) * 3;

  u32* pd = &m_rgb_data[m_rgb_pos_y * m_horizontal_size_value + m_rgb_pos_x];

  if (m_picture_coding_type == kPicTypeId_B) {
    if ((m_mb_type & kMBType_MotionForward) && (m_mb_type & kMBType_MotionBackward)) {

      for (int i = 0; i < 16; i++) {
        s32* pf0 = &m_wk_pic[1][src_offset[i & 1][0]];
        s32* pf1 = &m_wk_pic[1][src_offset[i & 1][0] + oddx[i & 1][0]];
        s32* pf2 = &m_wk_pic[1][src_offset[i & 1][0] + oddy[i & 1][0]];
        s32* pf3 = &m_wk_pic[1][src_offset[i & 1][0] + oddx[i & 1][0] + oddy[i & 1][0]];
        s32* pb0 = &m_wk_pic[0][src_offset[i & 1][1]];
        s32* pb1 = &m_wk_pic[0][src_offset[i & 1][1] + oddx[i & 1][1]];
        s32* pb2 = &m_wk_pic[0][src_offset[i & 1][1] + oddy[i & 1][1]];
        s32* pb3 = &m_wk_pic[0][src_offset[i & 1][1] + oddx[i & 1][1] + oddy[i & 1][1]];

        MACROBLOCK_LOG("[%d, %d]\n", m_rgb_pos_x, m_rgb_pos_y + i);
        for (int j = 0; j < 16; j++) {
          // 平均値を求める（小数部四捨五入）
          //s32 refy = (pf0[0] + pf1[0] + pf2[0] + pf3[0] + pb0[0] + pb1[0] + pb2[0] + pb3[0] + 4) >> 3;
          //s32 refu = (pf0[1] + pf1[1] + pf2[1] + pf3[1] + pb0[1] + pb1[1] + pb2[1] + pb3[1] + 4) >> 3;
          //s32 refv = (pf0[2] + pf1[2] + pf2[2] + pf3[2] + pb0[2] + pb1[2] + pb2[2] + pb3[2] + 4) >> 3;
          s32 reffy = (pf0[0] + pf1[0] + pf2[0] + pf3[0] + 2) >> 2;
          s32 reffu = (pf0[1] + pf1[1] + pf2[1] + pf3[1] + 2) >> 2;
          s32 reffv = (pf0[2] + pf1[2] + pf2[2] + pf3[2] + 2) >> 2;
          s32 refby = (pb0[0] + pb1[0] + pb2[0] + pb3[0] + 2) >> 2;
          s32 refbu = (pb0[1] + pb1[1] + pb2[1] + pb3[1] + 2) >> 2;
          s32 refbv = (pb0[2] + pb1[2] + pb2[2] + pb3[2] + 2) >> 2;
          s32 refy = (reffy + refby + 1) >> 1;
          s32 refu = (reffu + refbu + 1) >> 1;
          s32 refv = (reffv + refbv + 1) >> 1;
          pf0 += 3; pf1 += 3; pf2 += 3; pf3 += 3; pb0 += 3; pb1 += 3; pb2 += 3; pb3 += 3;

          s32 rgb[3];
          if (m_chroma_format >= kChromaFormat_4_2_0) {
            s32 y = m_yuv[0][i][j] + refy;
            s32 u = m_yuv[1][(i >> 1) & 7][(j >> 1) & 7] + refu;
            s32 v = m_yuv[2][(i >> 1) & 7][(j >> 1) & 7] + refv;
            // クリッピング(0～255)
            if (y < 0) y = 0; else if (y > 255) y = 255;
            if (u < 0) u = 0; else if (u > 255) u = 255;
            if (v < 0) v = 0; else if (v > 255) v = 255;

            ds_util::ycbcr2rgb(y, u, v, rgb[0], rgb[1], rgb[2]);
            MACROBLOCK_LOG("  (%3d %3d %3d) > (%3d %3d %3d)\n", y, u, v, rgb[0], rgb[1], rgb[2]);
          }
          pd[j] = (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
        }
        pd += m_horizontal_size_value;
        src_offset[i & 1][0] += width_bytes * 2;
        src_offset[i & 1][1] += width_bytes * 2;
      }
    } else if (m_mb_type & kMBType_MotionBackward) {
      for (int i = 0; i < 16; i++) {
        s32* pb0 = &m_wk_pic[0][src_offset[i & 1][1]];
        s32* pb1 = &m_wk_pic[0][src_offset[i & 1][1] + oddx[i & 1][1]];
        s32* pb2 = &m_wk_pic[0][src_offset[i & 1][1] + oddy[i & 1][1]];
        s32* pb3 = &m_wk_pic[0][src_offset[i & 1][1] + oddx[i & 1][1] + oddy[i & 1][1]];

        MACROBLOCK_LOG("[%d, %d]\n", m_rgb_pos_x, m_rgb_pos_y + i);
        for (int j = 0; j < 16; j++) {
          // 平均値を求める（小数部四捨五入）
          s32 refy = (pb0[0] + pb1[0] + pb2[0] + pb3[0] + 2) >> 2;
          s32 refu = (pb0[1] + pb1[1] + pb2[1] + pb3[1] + 2) >> 2;
          s32 refv = (pb0[2] + pb1[2] + pb2[2] + pb3[2] + 2) >> 2;
          pb0 += 3; pb1 += 3; pb2 += 3; pb3 += 3;

          s32 rgb[3];
          if (m_chroma_format >= kChromaFormat_4_2_0) {
            s32 y = m_yuv[0][i][j] + refy;
            s32 u = m_yuv[1][(i >> 1) & 7][(j >> 1) & 7] + refu;
            s32 v = m_yuv[2][(i >> 1) & 7][(j >> 1) & 7] + refv;
            // クリッピング(0～255)
            if (y < 0) y = 0; else if (y > 255) y = 255;
            if (u < 0) u = 0; else if (u > 255) u = 255;
            if (v < 0) v = 0; else if (v > 255) v = 255;

            ds_util::ycbcr2rgb(y, u, v, rgb[0], rgb[1], rgb[2]);
            MACROBLOCK_LOG("  (%3d %3d %3d) > (%3d %3d %3d)\n", y, u, v, rgb[0], rgb[1], rgb[2]);
          }
          pd[j] = (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
        }
        pd += m_horizontal_size_value;
        src_offset[i & 1][1] += width_bytes * 2;
      }
    } else if (m_mb_type & kMBType_MotionForward) {

      for (int i = 0; i < 16; i++) {
        s32* pf0 = &m_wk_pic[1][src_offset[i & 1][0]];
        s32* pf1 = &m_wk_pic[1][src_offset[i & 1][0] + oddx[i & 1][0]];
        s32* pf2 = &m_wk_pic[1][src_offset[i & 1][0] + oddy[i & 1][0]];
        s32* pf3 = &m_wk_pic[1][src_offset[i & 1][0] + oddx[i & 1][0] + oddy[i & 1][0]];

        MACROBLOCK_LOG("[%d, %d]\n", m_rgb_pos_x, m_rgb_pos_y + i);
        for (int j = 0; j < 16; j++) {
          // 平均値を求める（小数部四捨五入）
          s32 refy = (pf0[0] + pf1[0] + pf2[0] + pf3[0] + 2) >> 2;
          s32 refu = (pf0[1] + pf1[1] + pf2[1] + pf3[1] + 2) >> 2;
          s32 refv = (pf0[2] + pf1[2] + pf2[2] + pf3[2] + 2) >> 2;
          pf0 += 3; pf1 += 3; pf2 += 3; pf3 += 3;

          s32 rgb[3];
          if (m_chroma_format >= kChromaFormat_4_2_0) {
            s32 y = m_yuv[0][i][j] + refy;
            s32 u = m_yuv[1][(i >> 1) & 7][(j >> 1) & 7] + refu;
            s32 v = m_yuv[2][(i >> 1) & 7][(j >> 1) & 7] + refv;
            // クリッピング(0～255)
            if (y < 0) y = 0; else if (y > 255) y = 255;
            if (u < 0) u = 0; else if (u > 255) u = 255;
            if (v < 0) v = 0; else if (v > 255) v = 255;

            ds_util::ycbcr2rgb(y, u, v, rgb[0], rgb[1], rgb[2]);
            MACROBLOCK_LOG("  (%3d %3d %3d) > (%3d %3d %3d)\n", y, u, v, rgb[0], rgb[1], rgb[2]);
          }
          pd[j] = (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
        }
        pd += m_horizontal_size_value;
        src_offset[i & 1][0] += width_bytes * 2;
      }
    } else {
      for (int i = 0; i < 16; i++) {
        MACROBLOCK_LOG("[%d, %d]\n", m_rgb_pos_x, m_rgb_pos_y + i);
        for (int j = 0; j < 16; j++) {
          s32 rgb[3];
          if (m_chroma_format >= kChromaFormat_4_2_0) {
            s32 y = m_yuv[0][i][j];
            s32 u = m_yuv[1][(i >> 1) & 7][(j >> 1) & 7];
            s32 v = m_yuv[2][(i >> 1) & 7][(j >> 1) & 7];
            // クリッピング(0～255)
            if (y < 0) y = 0; else if (y > 255) y = 255;
            if (u < 0) u = 0; else if (u > 255) u = 255;
            if (v < 0) v = 0; else if (v > 255) v = 255;

            ds_util::ycbcr2rgb(y, u, v, rgb[0], rgb[1], rgb[2]);
            MACROBLOCK_LOG("  (%3d %3d %3d) > (%3d %3d %3d)\n", y, u, v, rgb[0], rgb[1], rgb[2]);
          }
          pd[j] = (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
        }
        pd += m_horizontal_size_value;
      }
    }
  } else if (m_picture_coding_type == kPicTypeId_I
      || m_picture_coding_type == kPicTypeId_P) {

    for (int i = 0; i < 16; i++) {
      s32* pf0 = &m_wk_pic[1][src_offset[i & 1][0]];
      s32* pf1 = &m_wk_pic[1][src_offset[i & 1][0] + oddx[i & 1][0]];
      s32* pf2 = &m_wk_pic[1][src_offset[i & 1][0] + oddy[i & 1][0]];
      s32* pf3 = &m_wk_pic[1][src_offset[i & 1][0] + oddx[i & 1][0] + oddy[i & 1][0]];
      s32* pw0 = &m_wk_pic[0][dst_offset];
      s32* pw1 = &m_wk_pic[1][dst_offset];

      MACROBLOCK_LOG("[%d, %d]\n", m_rgb_pos_x, m_rgb_pos_y + i);
      for (int j = 0; j < 16; j++) {
        s32 refy, refu, refv;
        if (m_mb_type & (kMBType_MotionForward | kMBType_Pattern)) {
          // 平均値を求める（小数部四捨五入）
          refy = (pf0[0] + pf1[0] + pf2[0] + pf3[0] + 2) >> 2;
          refu = (pf0[1] + pf1[1] + pf2[1] + pf3[1] + 2) >> 2;
          refv = (pf0[2] + pf1[2] + pf2[2] + pf3[2] + 2) >> 2;
          pf0 += 3; pf1 += 3; pf2 += 3; pf3 += 3;
        } else {
          refy = refu = refv = 0;
        }
        pw0[0] = m_yuv[0][i][j] + refy;
        pw0[1] = m_yuv[1][(i >> 1) & 7][(j >> 1) & 7] + refu;
        pw0[2] = m_yuv[2][(i >> 1) & 7][(j >> 1) & 7] + refv;
        // クリッピング(0～255)
        if (pw0[0] < 0) pw0[0] = 0; else if (pw0[0] > 255) pw0[0] = 255;
        if (pw0[1] < 0) pw0[1] = 0; else if (pw0[1] > 255) pw0[1] = 255;
        if (pw0[2] < 0) pw0[2] = 0; else if (pw0[2] > 255) pw0[2] = 255;

        if (m_ippic_count > 1) {
          s32 rgb[3];
          if (m_chroma_format >= kChromaFormat_4_2_0) {
            ds_util::ycbcr2rgb(pw1[0], pw1[1], pw1[2], rgb[0], rgb[1], rgb[2]);
            MACROBLOCK_LOG("  (%3d %3d %3d) > (%3d %3d %3d)\n", pw1[0], pw1[1], pw1[2], rgb[0], rgb[1], rgb[2]);
          }
          pd[j] = (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
        }
        pw0 += 3; pw1 += 3;
      }
      pd += m_horizontal_size_value;
      src_offset[i & 1][0] += width_bytes * 2;
      dst_offset           += width_bytes;
    }
  }
}

void Mpeg2Decoder::motion_vectors_decode(u8* &p_ptr, u32 &p_src_bit, int p_s) {
  if (m_motion_vector_count == 1) {
    if (m_mv_format == kMVFormat_Field && m_dual_prime != 1) {
      m_motion_vertical_field_select[0][p_s] = read_bits(m_buf, p_ptr, p_src_bit, 1);
      MACROBLOCK_LOG("  motion_vertical_field_select[%d][%d]=%d\n", 0, p_s, m_motion_vertical_field_select[0][p_s]);
    }
    motion_vector_decode(p_ptr, p_src_bit, 0, p_s);
  } else {
    m_motion_vertical_field_select[0][p_s] = read_bits(m_buf, p_ptr, p_src_bit, 1);
    MACROBLOCK_LOG("  motion_vertical_field_select[%d][%d]=%d\n", 0, p_s, m_motion_vertical_field_select[0][p_s]);
    
    motion_vector_decode(p_ptr, p_src_bit, 0, p_s);
    
    m_motion_vertical_field_select[1][p_s] = read_bits(m_buf, p_ptr, p_src_bit, 1);
    MACROBLOCK_LOG("  motion_vertical_field_select[%d][%d]=%d\n", 1, p_s, m_motion_vertical_field_select[1][p_s]);
    
    motion_vector_decode(p_ptr, p_src_bit, 1, p_s);
  }
}

void Mpeg2Decoder::motion_vector_decode(u8* &p_ptr, u32 &p_src_bit, int p_r, int p_s) {

  static const vlc_entry kMotionVLC11BitsTbl[] = {
    0xffe0, 0x0320, 11, -16,  0xffe0, 0x0360, 11, -15,  0xffe0, 0x03a0, 11, -14,  0xffe0, 0x03e0, 11, -13,
    0xffe0, 0x0420, 11, -12,  0xffe0, 0x0460, 11, -11,  0xffc0, 0x04c0, 10, -10,  0xffc0, 0x0540, 10,  -9,
    0xffc0, 0x05c0, 10,  -8,  0xff00, 0x0700,  8,  -7,  0xff00, 0x0900,  8,  -6,  0xff00, 0x0b00,  8,  -5,
    0xfe00, 0x0e00,  7,  -4,  0xf800, 0x1800,  5,  -3,  0xf000, 0x3000,  4,  -2,  0xe000, 0x6000,  3,  -1,
    0x8000, 0x8000,  1,   0,  0xe000, 0x4000,  3,   1,  0xf000, 0x2000,  4,   2,  0xf800, 0x1000,  5,   3,
    0xfe00, 0x0c00,  7,   4,  0xff00, 0x0a00,  8,   5,  0xff00, 0x0800,  8,   6,  0xff00, 0x0600,  8,   7,
    0xffc0, 0x0580, 10,   8,  0xffc0, 0x0500, 10,   9,  0xffc0, 0x0480, 10,  10,  0xffe0, 0x0440, 11,  11,
    0xffe0, 0x0400, 11,  12,  0xffe0, 0x03c0, 11,  13,  0xffe0, 0x0380, 11,  14,  0xffe0, 0x0340, 11,  15,
    0xffe0, 0x0300, 11,  16,  0x0000, 0x0000,  0,   0,
  };

  static const vlc_entry kDMVecVLCTbl[] = {
    0xc000, 0x8000, 2,  1,
    0x8000, 0x0000, 1,  0,
    0xc000, 0xc000, 2, -1,
    0x0000, 0x0000, 0,  0,
  };
  
  PICTURE_LOG("  motion[%d][%d]\n", p_r, p_s);

  // インデックス=p_r, 前方/後方=p_s, 水平/垂直=0
  m_motion_code[p_r][p_s][0] = vlc_decode(kMotionVLC11BitsTbl, m_buf, p_ptr, p_src_bit);
  PICTURE_LOG("    motion_code(h)=%d\n", m_motion_code[p_r][p_s][0]);

  if (m_f_code[p_s][0] > 0 && m_motion_code[p_r][p_s][0] != 0) {
    m_motion_residual[p_r][p_s][0] = read_bits(m_buf, p_ptr, p_src_bit, m_f_code[p_s][0] - 1);
    PICTURE_LOG("    motion_residual(h)=%d\n", m_motion_residual[p_r][p_s][0]);
  } else {
    m_motion_residual[p_r][p_s][0] = 0;
  }

  if (m_dual_prime == 1) {
    m_dmvector[0] = vlc_decode(kDMVecVLCTbl, m_buf, p_ptr, p_src_bit);
    PICTURE_LOG("    dm_vector(h)=%d\n", m_dmvector[0]);
  }

  // インデックス=p_r, 前方/後方=p_s, 水平/垂直=1
  m_motion_code[p_r][p_s][1] = vlc_decode(kMotionVLC11BitsTbl, m_buf, p_ptr, p_src_bit);
  PICTURE_LOG("    motion_code(v)=%d\n", m_motion_code[p_r][p_s][1]);

  if (m_f_code[p_s][1] > 0 && m_motion_code[p_r][p_s][1] != 0) {
    m_motion_residual[p_r][p_s][1] = read_bits(m_buf, p_ptr, p_src_bit, m_f_code[p_s][1] - 1);
    PICTURE_LOG("    motion_residual(v)=%d\n", m_motion_residual[p_r][p_s][1]);
  } else {
    m_motion_residual[p_r][p_s][1] = 0;
  }

  if (m_dual_prime == 1) {
    m_dmvector[1] = vlc_decode(kDMVecVLCTbl, m_buf, p_ptr, p_src_bit);
    PICTURE_LOG("    dm_vector(v)=%d\n", m_dmvector[1]);
  }
}

void Mpeg2Decoder::motion_vector_calc(bool p_fieldBasedPrediction) {
  int old_mv[2][2][2] = {
    m_motion_vector[0][0][0],
    m_motion_vector[0][0][1],
    m_motion_vector[0][1][0],
    m_motion_vector[0][1][1],
    m_motion_vector[1][0][0],
    m_motion_vector[1][0][1],
    m_motion_vector[1][1][0],
    m_motion_vector[1][1][1],
  };
  for (int i = 0; i < 8; i++) {
    if ((i & 2) == 0 && (m_mb_type & kMBType_MotionForward) == 0
    ||  (i & 2) == 2 && (m_mb_type & kMBType_MotionBackward) == 0) {
      continue;
    }

    const int f      = 1 << (m_f_code[i >> 1 & 1][i & 1] - 1);
    const int high    =  16 * f - 1;
    const int low    = -16 * f;
    const int range    =  32 * f;
    const int offset_v  = p_fieldBasedPrediction && (i & 1) ? 2 : 1;
    const int mv    = p_fieldBasedPrediction ? (i >> 2 & 1) : 0;

    int delta;
    if (m_motion_code[mv][i >> 1 & 1][i & 1] == 0 || f == 1) {
      delta = m_motion_code[mv][i >> 1 & 1][i & 1];
    } else {
      delta = (abs(m_motion_code[mv][i >> 1 & 1][i & 1]) - 1) * f + m_motion_residual[mv][i >> 1 & 1][i & 1] + 1;
      if (m_motion_code[mv][i >> 1 & 1][i & 1] < 0) {
        delta *= -1;
      }
    }
    // モジュロ補正
    // offset_vの除算でGPLではシフトを使用していたため差異が発生していた。これは除算で正しいと思う。
    // 一応完全一致が取れるまであわせておく
    int pred = old_mv[mv][i >> 1 & 1][i & 1] >> (offset_v - 1);
    int tmp = delta + pred;
    if (tmp < low) {
      tmp += range;
    }
    if (tmp > high) {
      tmp -= range;
    }
    // フレーム予測の場合、両方の動きベクトルを同じ値で書き換える
    m_motion_vector[i >> 2 & 1][i >> 1 & 1][i & 1] = tmp * offset_v;
  }
  PICTURE_LOG("  motion1(fw)=(%d,%d) > (%d,%d)\n", old_mv[0][0][0], old_mv[0][0][1], m_motion_vector[0][0][0], m_motion_vector[0][0][1]);
  if (p_fieldBasedPrediction) PICTURE_LOG("  motion2(fw)=(%d,%d) > (%d,%d)\n", old_mv[1][0][0], old_mv[1][0][1], m_motion_vector[1][0][0], m_motion_vector[1][0][1]);
  PICTURE_LOG("  motion1(bk)=(%d,%d) > (%d,%d)\n", old_mv[0][1][0], old_mv[0][1][1], m_motion_vector[0][1][0], m_motion_vector[0][1][1]);
  if (p_fieldBasedPrediction) PICTURE_LOG("  motion2(bk)=(%d,%d) > (%d,%d)\n", old_mv[1][1][0], old_mv[1][1][1], m_motion_vector[1][1][0], m_motion_vector[1][1][1]);
}

void Mpeg2Decoder::coded_block_pattern_decode(u8* &p_ptr, u32 &p_src_bit) {
  static const vlc_entry kCBPVLCTbl[] = {
    0xe000, 0xe000, 3, 60,  0xf000, 0xd000, 4,  4,  0xf000, 0xc000, 4,  8,  0xf000, 0xb000, 4, 16,
    0xf000, 0xa000, 4, 32,  0xf800, 0x9800, 5, 12,  0xf800, 0x9000, 5, 48,  0xf800, 0x8800, 5, 20,
    0xf800, 0x8000, 5, 40,  0xf800, 0x7800, 5, 28,  0xf800, 0x7000, 5, 44,  0xf800, 0x6800, 5, 52,
    0xf800, 0x6000, 5, 56,  0xf800, 0x5800, 5,  1,  0xf800, 0x5000, 5, 61,  0xf800, 0x4800, 5,  2,
    0xf800, 0x4000, 5, 62,  0xfc00, 0x3c00, 6, 24,  0xfc00, 0x3800, 6, 36,  0xfc00, 0x3400, 6,  3,
    0xfc00, 0x3000, 6, 63,  0xfe00, 0x2e00, 7,  5,  0xfe00, 0x2c00, 7,  9,  0xfe00, 0x2a00, 7, 17,
    0xfe00, 0x2800, 7, 33,  0xfe00, 0x2600, 7,  6,  0xfe00, 0x2400, 7, 10,  0xfe00, 0x2200, 7, 18,
    0xfe00, 0x2000, 7, 34,  0xff00, 0x1f00, 8,  7,  0xff00, 0x1e00, 8, 11,  0xff00, 0x1d00, 8, 19,
    0xff00, 0x1c00, 8, 35,  0xff00, 0x1b00, 8, 13,  0xff00, 0x1a00, 8, 49,  0xff00, 0x1900, 8, 21,
    0xff00, 0x1800, 8, 41,  0xff00, 0x1700, 8, 14,  0xff00, 0x1600, 8, 50,  0xff00, 0x1500, 8, 22,
    0xff00, 0x1400, 8, 42,  0xff00, 0x1300, 8, 15,  0xff00, 0x1200, 8, 51,  0xff00, 0x1100, 8, 23,
    0xff00, 0x1000, 8, 43,  0xff00, 0x0f00, 8, 25,  0xff00, 0x0e00, 8, 37,  0xff00, 0x0d00, 8, 26,
    0xff00, 0x0c00, 8, 38,  0xff00, 0x0b00, 8, 29,  0xff00, 0x0a00, 8, 45,  0xff00, 0x0900, 8, 53,
    0xff00, 0x0800, 8, 57,  0xff00, 0x0700, 8, 30,  0xff00, 0x0600, 8, 46,  0xff00, 0x0500, 8, 54,
    0xff00, 0x0400, 8, 58,  0xff80, 0x0380, 9, 31,  0xff80, 0x0300, 9, 47,  0xff80, 0x0280, 9, 55,
    0xff80, 0x0200, 9, 59,  0xff80, 0x0180, 9, 27,  0xff80, 0x0100, 9, 39,  0x0000, 0x0000, 0,  0,
  };

  for (int i = 0; i < 12; i++) {
    m_pattern_code[i] = 0;
  }

  if (m_mb_type & kMBType_Intra) {

    m_pattern_code[0] = m_pattern_code[1] = m_pattern_code[2] = m_pattern_code[3] = 1;
    if (m_chroma_format >= kChromaFormat_4_2_0) {
      m_pattern_code[4] = m_pattern_code[5] = 1;
    }
    if (m_chroma_format >= kChromaFormat_4_2_2) {
      m_pattern_code[6] = m_pattern_code[7] = 1;
    }
    if (m_chroma_format >= kChromaFormat_4_4_4) {
      m_pattern_code[8] = m_pattern_code[9] = m_pattern_code[10] = m_pattern_code[11] = 1;
    }

  } else {
    if (m_mb_type & kMBType_Pattern) {
      u32 cbp420 = vlc_decode(kCBPVLCTbl, m_buf, p_ptr, p_src_bit);
      MACROBLOCK_LOG("  coded_block_pattern_420=%d\n", cbp420);
      for (int j = 0; j < 6; j++) {
        if (cbp420 & (1 << (5 - j))) {
          m_pattern_code[j] = 1;
        }
      }

      if (m_chroma_format == kChromaFormat_4_2_2) {
        u32 coded_block_pattern_1 = read_bits(m_buf, p_ptr, p_src_bit, 2);
        MACROBLOCK_LOG("  coded_block_pattern_1=%d\n", coded_block_pattern_1);
        for (int j = 6; j < 8; j++) {
          if (coded_block_pattern_1 & (1 << (7 - j))) {
            m_pattern_code[j] = 1;
          }
        }
      } else if (m_chroma_format == kChromaFormat_4_4_4) {
        u32 coded_block_pattern_2 = read_bits(m_buf, p_ptr, p_src_bit, 6);
        MACROBLOCK_LOG("  coded_block_pattern_2=%d\n", coded_block_pattern_2);
        for (int j = 6; j < 12; j++) {
          if (coded_block_pattern_2 & (1 << (11 - j))) {
            m_pattern_code[j] = 1;
          }
        }
      }
    }
  }
}

bool Mpeg2Decoder::block_decode(u8* &p_ptr, u32 &p_src_bit, int p_i) {

  if (m_pattern_code[p_i] == 0) return false;

  int ch;
  if (p_i < 4) {  // 輝度
    ch = 0;
  } else {
    ch = 1 + (p_i & 1);
  }

  for (int i = 0; i <= 64; i++) {
    if (i == 0 && (m_mb_type & kMBType_Intra)) {
      // intra dc
      MACROBLOCK_LOG("intra dc\n");
      s32 dc_size;
      if (p_i < 4) {  // 輝度
        static const vlc_entry kDctDcLuVLCTbl[] = {
          0xe000, 0x8000, 3,  0,  0xc000, 0x0000, 2,  1,  0xc000, 0x4000, 2,  2,  0xe000, 0xa000, 3,  3,
          0xe000, 0xc000, 3,  4,  0xf000, 0xe000, 4,  5,  0xf800, 0xf000, 5,  6,  0xfc00, 0xf800, 6,  7,
          0xfe00, 0xfc00, 7,  8,  0xff00, 0xfe00, 8,  9,  0xff80, 0xff00, 9, 10,  0xff80, 0xff80, 9, 11,
        };
        dc_size = vlc_decode(kDctDcLuVLCTbl, m_buf, p_ptr, p_src_bit);
        MACROBLOCK_LOG("  dct_dc_size_luminance=%d\n", dc_size);
      } else {    // 色差
        static const vlc_entry kDctDcChVLCTbl[] = {
          0xc000, 0x0000,  2,  0,  0xc000, 0x4000,  2,  1,  0xc000, 0x8000,  2,  2,  0xe000, 0xc000,  3,  3,
          0xf000, 0xe000,  4,  4,  0xf800, 0xf000,  5,  5,  0xfc00, 0xf800,  6,  6,  0xfe00, 0xfc00,  7,  7,
          0xff00, 0xfe00,  8,  8,  0xff80, 0xff00,  9,  9,  0xffc0, 0xff80, 10, 10, 0xffc0, 0xffc0, 10, 11,
        };
        dc_size = vlc_decode(kDctDcChVLCTbl, m_buf, p_ptr, p_src_bit);
        MACROBLOCK_LOG("  dct_dc_size_chrominance=%d\n", dc_size);
      }

      m_dct_dc_differential = 0;
      if (dc_size != 0) {
        assert(dc_size < 12);
        s32 value = read_bits(m_buf, p_ptr, p_src_bit, dc_size);
        s32 range = 2 << (dc_size - 1);
        s32 half_range = range >> 1;
        s32 mask = range - 1;
        m_dct_dc_differential = value < half_range ? -(~value & mask) : value;
        MACROBLOCK_LOG("  dct_dc_differential=%d\n", m_dct_dc_differential);
      }
      m_qfs[0] = m_dct_dc_pred[ch] + m_dct_dc_differential;
      m_dct_dc_pred[ch] = m_qfs[0];
      //MACROBLOCK_LOG("qfs=%d\n", m_qfs[0]);
    } else {
      // intra ac / non intra
      if (i == 0) MACROBLOCK_LOG("intra ac / non intra\n");
      static const runlength_vlc_entry kDCTCoeffVLCTblFmt[2][114] = {
        // format0
        0x8000,  2,  0,  1,  0xc000,  2,  0,  1,  0x6000,  3,  1,  1,  0x4000,  4,  0,  2,  0x5000,  4,  2,  1,  0x2800,  5,  0,  3,  0x3800,  5,  3,  1,  0x3000,  5,  4,  1,
        0x1800,  6,  1,  2,  0x1c00,  6,  5,  1,  0x1400,  6,  6,  1,  0x1000,  6,  7,  1,  0x0c00,  7,  0,  4,  0x0800,  7,  2,  2,  0x0e00,  7,  8,  1,  0x0a00,  7,  9,  1,
        0x0400,  6, -1,  0,
        0x2600,  8,  0,  5,  0x2100,  8,  0,  6,  0x2500,  8,  1,  3,  0x2400,  8,  3,  2,  0x2700,  8, 10,  1,  0x2300,  8, 11,  1,  0x2200,  8, 12,  1,  0x2000,  8, 13,  1,
        0x0280, 10,  0,  7,  0x0300, 10,  1,  4,  0x02c0, 10,  2,  3,  0x03c0, 10,  4,  2,  0x0240, 10,  5,  2,  0x0380, 10, 14,  1,  0x0340, 10, 15,  1,  0x0200, 10, 16,  1,
        0x01d0, 12,  0,  8,  0x0180, 12,  0,  9,  0x0130, 12,  0, 10,  0x0100, 12,  0, 11,  0x01b0, 12,  1,  5,  0x0140, 12,  2,  4,  0x01c0, 12,  3,  3,  0x0120, 12,  4,  3,
        0x01e0, 12,  6,  2,  0x0150, 12,  7,  2,  0x0110, 12,  8,  2,  0x01f0, 12, 17,  1,  0x01a0, 12, 18,  1,  0x0190, 12, 19,  1,  0x0170, 12, 20,  1,  0x0160, 12, 21,  1,
        0x00d0, 13,  0, 12,  0x00c8, 13,  0, 13,  0x00c0, 13,  0, 14,  0x00b8, 13,  0, 15,  0x00b0, 13,  1,  6,  0x00a8, 13,  1,  7,  0x00a0, 13,  2,  5,  0x0098, 13,  3,  4,
        0x0090, 13,  5,  3,  0x0088, 13,  9,  2,  0x0080, 13, 10,  2,  0x00f8, 13, 22,  1,  0x00f0, 13, 23,  1,  0x00e8, 13, 24,  1,  0x00e0, 13, 25,  1,  0x00d8, 13, 26,  1,
        0x007c, 14,  0, 16,  0x0078, 14,  0, 17,  0x0074, 14,  0, 18,  0x0070, 14,  0, 19,  0x006c, 14,  0, 20,  0x0068, 14,  0, 21,  0x0064, 14,  0, 22,  0x0060, 14,  0, 23,
        0x005c, 14,  0, 24,  0x0058, 14,  0, 25,  0x0054, 14,  0, 26,  0x0050, 14,  0, 27,  0x004c, 14,  0, 28,  0x0048, 14,  0, 29,  0x0044, 14,  0, 30,  0x0040, 14,  0, 31,
        0x0030, 15,  0, 32,  0x002e, 15,  0, 33,  0x002c, 15,  0, 34,  0x002a, 15,  0, 35,  0x0028, 15,  0, 36,  0x0026, 15,  0, 37,  0x0024, 15,  0, 38,  0x0022, 15,  0, 39,
        0x0020, 15,  0, 40,  0x003e, 15,  1,  8,  0x003c, 15,  1,  9,  0x003a, 15,  1, 10,  0x0038, 15,  1, 11,  0x0036, 15,  1, 12,  0x0034, 15,  1, 13,  0x0032, 15,  1, 14,
        0x0013, 16,  1, 15,  0x0012, 16,  1, 16,  0x0011, 16,  1, 17,  0x0010, 16,  1, 18,  0x0014, 16,  6,  3,  0x001a, 16, 11,  2,  0x0019, 16, 12,  2,  0x0018, 16, 13,  2,
        0x0017, 16, 14,  2,  0x0016, 16, 15,  2,  0x0015, 16, 16,  2,  0x001f, 16, 27,  1,  0x001e, 16, 28,  1,  0x001d, 16, 29,  1,  0x001c, 16, 30,  1,  0x001b, 16, 31,  1,
        0x0000,  0,  0,  0,
        // format1
        0x6000,  4,  0,  0,  0x8000,  2,  0,  1,  0x4000,  3,  1,  1,  0xc000,  3,  0,  2,  0x2800,  5,  2,  1,  0x7000,  4,  0,  3,  0x3800,  5,  3,  1,  0x1800,  6,  4,  1,
        0x3000,  5,  1,  2,  0x1c00,  6,  5,  1,  0x0c00,  7,  6,  1,  0x0800,  7,  7,  1,  0xe000,  5,  0,  4,  0x0e00,  7,  2,  2,  0x0a00,  7,  8,  1,  0xf000,  7,  9,  1,
        0x0400,  6, -1,  0,
        0xe800,  5,  0,  5,  0x1400,  6,  0,  6,  0xf200,  7,  1,  3,  0x2600,  8,  3,  2,  0xf400,  7, 10,  1,  0x2100,  8, 11,  1,  0x2500,  8, 12,  1,  0x2400,  8, 13,  1,
        0x1000,  6,  0,  7,  0x2700,  8,  1,  4,  0xfc00,  8,  2,  3,  0xfd00,  8,  4,  2,  0x0200,  9,  5,  2,  0x0280,  9, 14,  1,  0x0380,  9, 15,  1,  0x0340, 10, 16,  1,
        0xf600,  7,  0,  8,  0xf800,  7,  0,  9,  0x2300,  8,  0, 10,  0x2200,  8,  0, 11,  0x2000,  8,  1,  5,  0x0300, 10,  2,  4,  0x01c0, 12,  3,  3,  0x0120, 12,  4,  3,
        0x01e0, 12,  6,  2,  0x0150, 12,  7,  2,  0x0110, 12,  8,  2,  0x01f0, 12, 17,  1,  0x01a0, 12, 18,  1,  0x0190, 12, 19,  1,  0x0170, 12, 20,  1,  0x0160, 12, 21,  1,
        0xfa00,  8,  0, 12,  0xfb00,  8,  0, 13,  0xfe00,  8,  0, 14,  0xff00,  8,  0, 15,  0x00b0, 13,  1,  6,  0x00a8, 13,  1,  7,  0x00a0, 13,  2,  5,  0x0098, 13,  3,  4,
        0x0090, 13,  5,  3,  0x0088, 13,  9,  2,  0x0080, 13, 10,  2,  0x00f8, 13, 22,  1,  0x00f0, 13, 23,  1,  0x00e8, 13, 24,  1,  0x00e0, 13, 25,  1,  0x00d8, 13, 26,  1,
        0x007c, 14,  0, 16,  0x0078, 14,  0, 17,  0x0074, 14,  0, 18,  0x0070, 14,  0, 19,  0x006c, 14,  0, 20,  0x0068, 14,  0, 21,  0x0064, 14,  0, 22,  0x0060, 14,  0, 23,
        0x005c, 14,  0, 24,  0x0058, 14,  0, 25,  0x0054, 14,  0, 26,  0x0050, 14,  0, 27,  0x004c, 14,  0, 28,  0x0048, 14,  0, 29,  0x0044, 14,  0, 30,  0x0040, 14,  0, 31,
        0x0030, 15,  0, 32,  0x002e, 15,  0, 33,  0x002c, 15,  0, 34,  0x002a, 15,  0, 35,  0x0028, 15,  0, 36,  0x0026, 15,  0, 37,  0x0024, 15,  0, 38,  0x0022, 15,  0, 39,
        0x0020, 15,  0, 40,  0x003e, 15,  1,  8,  0x003c, 15,  1,  9,  0x003a, 15,  1, 10,  0x0038, 15,  1, 11,  0x0036, 15,  1, 12,  0x0034, 15,  1, 13,  0x0032, 15,  1, 14,
        0x0013, 16,  1, 15,  0x0012, 16,  1, 16,  0x0011, 16,  1, 17,  0x0010, 16,  1, 18,  0x0014, 16,  6,  3,  0x001a, 16, 11,  2,  0x0019, 16, 12,  2,  0x0018, 16, 13,  2,
        0x0017, 16, 14,  2,  0x0016, 16, 15,  2,  0x0015, 16, 16,  2,  0x001f, 16, 27,  1,  0x001e, 16, 28,  1,  0x001d, 16, 29,  1,  0x001c, 16, 30,  1,  0x001b, 16, 31,  1,
        0x0000,  0,  0,  0,
      };

      int dct_coeff_type = (m_mb_type & kMBType_Intra) && (m_intra_vlc_format == 1);
      //LOG("    dct_coeff_format=%d\n", dct_coeff_type);

      u32  vlc = fetch_bits(m_buf, p_ptr, p_src_bit, 16);

      runlength_vlc_entry entry;
      bool result = false;
      for (int j = 0; kDCTCoeffVLCTblFmt[dct_coeff_type][j].code; j++) {
        if (j == 0 && i == 0) {
          // dct_coeff_type=0のDCはEOBと11sの代わりに1sをrun=0,lv=1とみなす
          entry.code = 0x8000;
          entry.len = 1;
          entry.run = 0;
          entry.lv = 1;
        } else {
          entry = kDCTCoeffVLCTblFmt[dct_coeff_type][j];
        }

        u16 mask = 0xffff << (16 - entry.len);
        if ((vlc & mask) != entry.code) {
          continue;
        }

        if (j == 0 && i > 0) {
          // EOB

          // 一致したビット長分、実際に進める
          read_bits(m_buf, p_ptr, p_src_bit, entry.len);

          MACROBLOCK_LOG("    [%2d] %2d:eob\n", i, j);

          while (i < 64) {
            m_qfs[i++] = 0;
          }
          return false;
        } else if (entry.run == -1) {
          // エスケープ

          // 一致したビット長分、実際に進める
          read_bits(m_buf, p_ptr, p_src_bit, entry.len);

          entry.run = (s8)read_bits(m_buf, p_ptr, p_src_bit, 6);
          entry.lv  = (u16)read_bits(m_buf, p_ptr, p_src_bit, 12);
          if (entry.lv & 0x800) entry.lv = -0x1000 + entry.lv;
          
          MACROBLOCK_LOG("    [%2d] %2d(escape):run=%d level=%d\n", i + entry.run, j, entry.run, entry.lv);
          result = true;
          break;
        } else {
          // 符合を取得しつつ、一致したビット長分、実際に進める
          bool sign = !!(read_bits(m_buf, p_ptr, p_src_bit, entry.len + 1) & 1);
          entry.lv  = sign ? -entry.lv : entry.lv;

          MACROBLOCK_LOG("    [%2d] %2d:run=%d level=%d vlc=%04x len=%d\n", i + entry.run, j, entry.run, entry.lv, vlc, entry.len);
          result = true;
          break;
        }
      }

      if (result) {
        for (int k = 0; k < entry.run; k++) {
          m_qfs[i++] = 0;
        }
        m_qfs[i] = entry.lv;
        if (i == 0) {
          //MACROBLOCK_LOG("qfs=%d\n", m_qfs[0]);
        }
      } else {
        // VLCのビット列一致しない、デコードエラー
        MACROBLOCK_LOG("decode failed vlc=%04x\n", vlc);
        assert(0);
        return false;
      }
    }
  }
  // DCT値64個以後もEOBが見つからない場合、デコードエラー
  //MACROBLOCK_LOG("decode failed! vlc=%04x\n", vlc);
  assert(0);
  return false;
}

void Mpeg2Decoder::inverse_zigzag() {
  const int kInvZigzagTable[2][8][8] = {
    // zigzag
    0,  1,  5,  6,  14,  15,  27,  28,
    2,  4,  7,  13,  16,  26,  29,  42,
    3,  8,  12,  17,  25,  30,  41,  43,
    9,  11,  18,  24,  31,  40,  44,  53,
    10,  19,  23,  32,  39,  45,  52,  54,
    20,  22,  33,  38,  46,  51,  55,  60,
    21,  34,  37,  47,  50,  56,  59,  61,
    35,  36,  48,  49,  57,  58,  62,  63,
    // alternate
    0,  4,  6,  20,  22,  36,  38,  52,
    1,  5,  7,  21,  23,  37,  39,  53,
    2,  8,  19,  24,  34,  40,  50,  54,
    3,  9,  18,  25,  35,  41,  51,  55,
    10,  17,  26,  30,  42,  46,  56,  60,
    11,  16,  27,  31,  43,  47,  57,  61,
    12,  15,  28,  32,  44,  48,  58,  62,
    13,  14,  29,  33,  45,  49,  59,  63,
  };
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      m_qf[i][j] = m_qfs[kInvZigzagTable[m_alternate_scan][i][j]];
    }
  }
}

void Mpeg2Decoder::dequantize(int p_idx) {
  static const int kQuantizerScaleTable[2][32] = {
    0,  2,  4,  6,  8,  10,  12,  14,
    16,  18,  20,  22,  24,  26,  28,  30,
    32,  34,  36,  38,  40,  42,  44,  46,
    48,  50,  52,  54,  56,  58,  60,  62,
    0,  1,  2,  3,  4,  5,  6,  7,
    8,  10,  12,  14,  16,  18,  20,  22,
    24,  28,  32,  36,  40,  44,  48,  52,
    56,  64,  72,  80,  88,  96,  104,112,
  };

  s32  coeff_sum = 0;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      s32 quantizer_scale = -1;
      if (m_mb_type & kMBType_Intra) {
        // intra
        if (i == 0 && j == 0) {
          // dc
          m_f[0][0] = m_qf[0][0] * (8 >> m_intra_dc_precision);

          PICTURE_LOG("intra dc=%d\n", m_f[0][0]);
        } else {
          // ac
          quantizer_scale = kQuantizerScaleTable[m_q_scale_type][m_quantizer_scale_code];
          m_f[i][j] = 2 * m_qf[i][j] * m_intra_quantizer_matrix[i * 8 + j] * quantizer_scale / 32;
        }
      } else {
        // non intra
        s32 sign = (m_qf[i][j] == 0 ? 0 : (m_qf[i][j] > 0 ? 1 : -1));
        quantizer_scale = kQuantizerScaleTable[m_q_scale_type][m_quantizer_scale_code];
        m_f[i][j] = (2 * m_qf[i][j] + sign) * m_non_intra_quantizer_matrix[i * 8 + j] * quantizer_scale / 32;
      }
      // 飽和制御
      if (m_f[i][j] > 2047) {
        m_f[i][j] = 2047;
      } else if (m_f[i][j] < -2048) {
        m_f[i][j] = -2048;
      }
      coeff_sum += m_f[i][j];
    }
  }

  // ミスマッチ制御
  if ((coeff_sum & 1) == 0) {
    m_f[7][7] += (m_f[7][7] & 1) ? -1 : 1;
  }
}

void Mpeg2Decoder::decode_dct(int p_idx) {
  // 逆DCT
  int out[64];
	idct2d_88((int*)m_f, out);

  // Y   | Cb  | Cr
  //-----+-----+-----
  // 0 1 | 4 8 | 5 9
  // 2 3 | 6 a | 7 b
  int ch, x, y, stride;
  if (p_idx < 4) {
    // L
    ch = 0;
    x  = (p_idx & 1) * 8;

    if (m_dct_type == 1) {  // フィールドDCT
      y = (p_idx & 2) / 2;
      stride = 2;
    } else {        // フレームDCT
      y = (p_idx & 2) * 4;
      stride = 1;
    }
  } else {
    // Cb,Cr
    ch = (p_idx & 1) + 1;
    x  = (p_idx & 8);
    y  = (p_idx & 2) * 4;
    stride = 1;
  }
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      assert(y + i * stride < 16);
      assert(x + j < 16);
      assert(ch < 3);
      // クリッピング(-256～255)
      if (out[i * 8 + j] > 255) {
        m_yuv[ch][y + i * stride][x + j] = 255;
      } else if (out[i * 8 + j] < -256) {
        m_yuv[ch][y + i * stride][x + j] = -256;
      } else {
        m_yuv[ch][y + i * stride][x + j] = out[i * 8 + j];
      }
    }
  }
}

void Mpeg2Decoder::clear_yuvdata(int p_idx) {
  // Y   | Cb  | Cr
  //-----+-----+-----
  // 0 1 | 4 8 | 5 9
  // 2 3 | 6 a | 7 b
  int ch, x, y, stride;
  if (p_idx < 4) {
    // L
    ch = 0;
    x  = (p_idx & 1) * 8;

    if (m_dct_type == 1) {  // フィールドDCT
      y = (p_idx & 2) / 2;
      stride = 2;
    } else {        // フレームDCT
      y = (p_idx & 2) * 4;
      stride = 1;
    }
  } else {
    // Cb,Cr
    ch = (p_idx & 1) + 1;
    x  = (p_idx & 8);
    y  = (p_idx & 2) * 4;
    stride = 1;
  }

  /*
  const int stride16 = stride * 16;
  s32* ptr = &m_yuv[ch][y][x];
  for (int i = 0; i < 8; i++) {
    //memset(ptr, 0, sizeof(s32) * 8);
    ptr[0] = ptr[1] = ptr[2] = ptr[3] = ptr[4] = ptr[5] = ptr[6] = ptr[7] = 0;
    ptr += stride16;
  }
  */
  s32* p = &m_yuv[ch][y][x];
  _asm {
    mov edx, dword ptr[stride]
    shl edx, 6  // *= 64
    mov eax, dword ptr[p]
    pxor xmm0, xmm0
    mov ecx, 8
_loop:    movdqa oword ptr [eax], xmm0
    movdqa oword ptr [eax + 16], xmm0
    add eax, edx
    dec ecx
    jne _loop
  }
}

void Mpeg2Decoder::idct2d_88(int* p_in, int* p_out) {
#if defined _DEBUG
#define SHIFT_AMOUNT(_x) (int)(log(_x) / log(2.0) + 0.5)
  const int CALC_SCALE = 8;
  const int CALC_SCALE_SA = SHIFT_AMOUNT((double)CALC_SCALE);
  const int COS_SCALE_SA = 12;
  const int W0 = (int)(cos(M_PI / (2 * 8) * 0) * (1 << COS_SCALE_SA) * CALC_SCALE);
  const int W1 = (int)(cos(M_PI / (2 * 8) * 1) * (1 << COS_SCALE_SA) * CALC_SCALE * sqrt(2.0));
  const int W2 = (int)(cos(M_PI / (2 * 8) * 2) * (1 << COS_SCALE_SA) * CALC_SCALE * sqrt(2.0));
  const int W3 = (int)(cos(M_PI / (2 * 8) * 3) * (1 << COS_SCALE_SA) * CALC_SCALE * sqrt(2.0));
  const int W4 = (int)(cos(M_PI / (2 * 8) * 4) * (1 << COS_SCALE_SA) * CALC_SCALE * sqrt(2.0));
  const int W5 = (int)(cos(M_PI / (2 * 8) * 5) * (1 << COS_SCALE_SA) * CALC_SCALE * sqrt(2.0));
  const int W6 = (int)(cos(M_PI / (2 * 8) * 6) * (1 << COS_SCALE_SA) * CALC_SCALE * sqrt(2.0));
  const int W7 = (int)(cos(M_PI / (2 * 8) * 7) * (1 << COS_SCALE_SA) * CALC_SCALE * sqrt(2.0));
  const int DCT_COEFF = SHIFT_AMOUNT(sqrt(8.0) * sqrt(8.0));
  const int ROUNDING_UNIT = CALC_SCALE >> 1 << COS_SCALE_SA;
#undef SHIFT_AMOUNT
#else
  const int CALC_SCALE = 8;
  const int CALC_SCALE_SA = 3;
  const int COS_SCALE_SA = 12;
  const int W0 = 32768;
  const int W1 = 45450;
  const int W2 = 42813;
  const int W3 = 38531;
  const int W4 = 32768;
  const int W5 = 25745;
  const int W6 = 17733;
  const int W7 = 9040;
  const int DCT_COEFF = 3;
  const int ROUNDING_UNIT = 16384;
#endif

  //static const int CT[] = {
  //  _cos(0),  _cos(1),  _cos(2),  _cos(3),  _cos(4),  _cos(5),  _cos(6),  _cos(7),
  //  _cos(0),  _cos(3),  _cos(6), -_cos(7), -_cos(4), -_cos(1), -_cos(2), -_cos(5),
  //  _cos(0),  _cos(5), -_cos(6), -_cos(1), -_cos(4),  _cos(7),  _cos(2),  _cos(3),
  //  _cos(0),  _cos(7), -_cos(2), -_cos(5),  _cos(4),  _cos(3), -_cos(6), -_cos(1),
  //  _cos(0), -_cos(7), -_cos(2),  _cos(5),  _cos(4), -_cos(3), -_cos(6),  _cos(1),
  //  _cos(0), -_cos(5), -_cos(6),  _cos(1), -_cos(4), -_cos(7),  _cos(2), -_cos(3),
  //  _cos(0), -_cos(3),  _cos(6),  _cos(7), -_cos(4),  _cos(1), -_cos(2),  _cos(5),
  //  _cos(0), -_cos(1),  _cos(2), -_cos(3),  _cos(4), -_cos(5),  _cos(6), -_cos(7),
  //};

  int* ip;
  int* op;

  ip = p_in;
  op = p_out;

  for (int y = 0; y < 8; y++) {
    int a0 = ip[0] * W0 + ROUNDING_UNIT;
    int b0 = ip[1] * W1;
    int b1 = ip[1] * W3;
    int b2 = ip[1] * W5;
    int b3 = ip[1] * W7;
    int c0 = ip[2] * W2;
    int c1 = ip[2] * W6;
    int d0 = ip[3] * W1;
    int d1 = ip[3] * W3;
    int d2 = ip[3] * W5;
    int d3 = ip[3] * W7;
    int e0 = ip[4] * W4;
    int f0 = ip[5] * W1;
    int f1 = ip[5] * W3;
    int f2 = ip[5] * W5;
    int f3 = ip[5] * W7;
    int g0 = ip[6] * W2;
    int g1 = ip[6] * W6;
    int h0 = ip[7] * W1;
    int h1 = ip[7] * W3;
    int h2 = ip[7] * W5;
    int h3 = ip[7] * W7;
    
    int ae0 = a0 + e0;
    int ae1 = a0 - e0;
    int cg0 = c0 + g1;
    int cg1 = c1 - g0;
    int bd0 = b0 + d1;
    int bd1 = b1 - d3;
    int bd2 = b2 - d0;
    int bd3 = b3 - d2;
    int fh0 = ip[5] * W1 + ip[7] * W5;
    int fh1 = ip[5] * W3 - ip[7] * W1;
    int fh2 = ip[5] * W5 + ip[7] * W7;
    int fh3 = ip[5] * W7 + ip[7] * W3;
    
    int aecg0 = ae0 + cg0;
    int aecg1 = ae0 - cg0;
    int aecg2 = ae1 + cg1;
    int aecg3 = ae1 - cg1;
    int bdfh0 = bd0 + fh2;
    int bdfh1 = bd1 - fh0;
    int bdfh2 = bd2 + fh3;
    int bdfh3 = bd3 + fh1;
    op[0] = (aecg0 + bdfh0) >> (COS_SCALE_SA + CALC_SCALE_SA);
    op[1] = (aecg2 + bdfh1) >> (COS_SCALE_SA + CALC_SCALE_SA);
    op[2] = (aecg3 + bdfh2) >> (COS_SCALE_SA + CALC_SCALE_SA);
    op[3] = (aecg1 + bdfh3) >> (COS_SCALE_SA + CALC_SCALE_SA);
    op[4] = (aecg1 - bdfh3) >> (COS_SCALE_SA + CALC_SCALE_SA);
    op[5] = (aecg3 - bdfh2) >> (COS_SCALE_SA + CALC_SCALE_SA);
    op[6] = (aecg2 - bdfh1) >> (COS_SCALE_SA + CALC_SCALE_SA);
    op[7] = (aecg0 - bdfh0) >> (COS_SCALE_SA + CALC_SCALE_SA);
    ip += 8;
    op += 8;
  }

  ip = p_in;
  op = p_out;

  for (int y = 0; y < 8; y++) {
    int a0 = op[0]  * W0 + (ROUNDING_UNIT << DCT_COEFF);
    int b0 = op[8]  * W1;
    int b1 = op[8]  * W3;
    int b2 = op[8]  * W5;
    int b3 = op[8]  * W7;
    int c0 = op[16] * W2;
    int c1 = op[16] * W6;
    int d0 = op[24] * W1;
    int d1 = op[24] * W3;
    int d2 = op[24] * W5;
    int d3 = op[24] * W7;
    int e0 = op[32] * W4;
    int f0 = op[40] * W1;
    int f1 = op[40] * W3;
    int f2 = op[40] * W5;
    int f3 = op[40] * W7;
    int g0 = op[48] * W2;
    int g1 = op[48] * W6;
    int h0 = op[56] * W1;
    int h1 = op[56] * W3;
    int h2 = op[56] * W5;
    int h3 = op[56] * W7;
    
    int ae0 = a0 + e0;
    int ae1 = a0 - e0;
    int cg0 = c0 + g1;
    int cg1 = c1 - g0;
    int bd0 = b0 + d1;
    int bd1 = b1 - d3;
    int bd2 = b2 - d0;
    int bd3 = b3 - d2;
    int fh0 = f0 + h2;
    int fh1 = f1 - h0;
    int fh2 = f2 + h3;
    int fh3 = f3 + h1;
    
    int aecg0 = ae0 + cg0;
    int aecg1 = ae0 - cg0;
    int aecg2 = ae1 + cg1;
    int aecg3 = ae1 - cg1;
    int bdfh0 = bd0 + fh2;
    int bdfh1 = bd1 - fh0;
    int bdfh2 = bd2 + fh3;
    int bdfh3 = bd3 + fh1;
    op[0]  = (aecg0 + bdfh0) >> (COS_SCALE_SA + DCT_COEFF + CALC_SCALE_SA);
    op[8]  = (aecg2 + bdfh1) >> (COS_SCALE_SA + DCT_COEFF + CALC_SCALE_SA);
    op[16] = (aecg3 + bdfh2) >> (COS_SCALE_SA + DCT_COEFF + CALC_SCALE_SA);
    op[24] = (aecg1 + bdfh3) >> (COS_SCALE_SA + DCT_COEFF + CALC_SCALE_SA);
    op[32] = (aecg1 - bdfh3) >> (COS_SCALE_SA + DCT_COEFF + CALC_SCALE_SA);
    op[40] = (aecg3 - bdfh2) >> (COS_SCALE_SA + DCT_COEFF + CALC_SCALE_SA);
    op[48] = (aecg2 - bdfh1) >> (COS_SCALE_SA + DCT_COEFF + CALC_SCALE_SA);
    op[56] = (aecg0 - bdfh0) >> (COS_SCALE_SA + DCT_COEFF + CALC_SCALE_SA);
    ip++;
    op++;
  }
}
