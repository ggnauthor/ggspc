#if defined _UNIT_TEST

/*---------*/
/* include */
/*---------*/
#include "memory/memory.h"
#include "device/graphic.h"
#include "device/audio.h"

/*--------*/
/* define */
/*--------*/

/*--------*/
/* global */
/*--------*/

/*----------*/
/* function */
/*----------*/
FIXTURE(ggsext_fix_01);

SETUP(ggsext_fix_01) {
}

TEARDOWN(ggsext_fix_01) {
}

void decode5_sample(u8* p_dst, u8* p_src) {
  static bool loaded = false;
  static u8 decode5a[0x0dac];

  DWORD oldProtect;
  VirtualProtect(decode5a, 0x0dac, PAGE_EXECUTE_READWRITE, &oldProtect);

  if (loaded == false) {
    // ここで落ちたら作業ディレクトリに ../. を指定すること
    FILE *fp = fopen("memo/ggxx.exe", "rb");
    fseek(fp, 0x001b20, SEEK_SET);
    fread(&decode5a, 1, 0x0dac, fp);
    fclose(fp);
  }

  __asm {
    lea eax, decode5a
    push p_src
    push p_dst
    call eax
    add esp, 8
  }
}

void image_decode_test(u32 p_img_base_ofs) {

  FILE *fp = fopen("data/SLPM_663.33/SCDDATA.BIN", "rb");
  if (fp) {
    u32 ofs1, ofs2;

    fseek(fp, p_img_base_ofs, SEEK_SET);
    fread(&ofs1, 1, sizeof(u32), fp);

    fseek(fp, p_img_base_ofs + ofs1 + 4, SEEK_SET);
    fread(&ofs2, 1, sizeof(u32), fp);

    fseek(fp, p_img_base_ofs + ofs1 + ofs2, SEEK_SET);
    int image_list_array[2048];
    int image_list_size = fread(&image_list_array, 1, 4 * 2048, fp);

    for (int i = 0; i < image_list_size / 4; i++) {
      if (image_list_array[i] == 0xFFFFFFFF) break;
      u32  offset = image_list_array[i] + p_img_base_ofs + ofs1 + ofs2;
      
      char  header[16];
      fseek(fp, offset, SEEK_SET);
      fread(header, 1, 16, fp);
      int palsize = 1 << header[4];
      bool fixed_pal = (header[2] == 0x20);
      int  image_w = *((WORD*)&header[6]);
      int  image_h = *((WORD*)&header[8]);

      // パレットなどがあれば飛ばす
      if (fixed_pal) {
        fseek(fp, palsize * 4, SEEK_CUR);
      }

      int enc_image_size = image_w * image_h * 2;
      u8*  src_buf = new u8[enc_image_size];
      fread(src_buf, 1, enc_image_size, fp);

      int dec_image_size = image_w * image_h * 4;
      u8*  dst_buf = new u8[dec_image_size];
      memset(dst_buf, 0, dec_image_size);

      u8* cor_buf = new u8[dec_image_size];
      memset(cor_buf, 0, dec_image_size);

      u8* ptr = dst_buf;
      GraphicMgr::decode5(dst_buf, src_buf);
      ptr = cor_buf;
      decode5_sample(ptr, src_buf);
      for (int j = 0; j < dec_image_size; j++) {
        if (cor_buf[j] != dst_buf[j]) {
          WIN_ASSERT_FAIL("decode error idx=%d, ofs=%d", i, j);
        }
      }
      delete[] src_buf;
      delete[] dst_buf;
      delete[] cor_buf;
    }
    fclose(fp);
  }
}

BEGIN_TESTF(ggsext_decode5_so, ggsext_fix_01){
  try {
    image_decode_test(0x9a79800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_ky, ggsext_fix_01){
  try {
    image_decode_test(0x9e5d000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_ma, ggsext_fix_01){
  try {
    image_decode_test(0xa221000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_mi, ggsext_fix_01){
  try {
    image_decode_test(0xa62b000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_ed, ggsext_fix_01){
  try {
    image_decode_test(0xb3fc000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_po, ggsext_fix_01){
  try {
    image_decode_test(0xac56800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_ch, ggsext_fix_01){
  try {
    image_decode_test(0xb117000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_fa, ggsext_fix_01){
  try {
    image_decode_test(0xba0f800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_ax, ggsext_fix_01){
  try {
    image_decode_test(0xa8d6800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_ba, ggsext_fix_01){
  try {
    image_decode_test(0xb6fa800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_an, ggsext_fix_01){
  try {
    image_decode_test(0xc5e9800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_ve, ggsext_fix_01){
  try {
    image_decode_test(0xcd84800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_jo, ggsext_fix_01){
  try {
    image_decode_test(0xc9ec000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_ja, ggsext_fix_01){
  try {
    image_decode_test(0xc27c800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_te, ggsext_fix_01){
  try {
    image_decode_test(0xbe3f000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_di, ggsext_fix_01){
  try {
    image_decode_test(0xd002800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_br, ggsext_fix_01){
  try {
    image_decode_test(0xe456800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_sl, ggsext_fix_01){
  try {
    image_decode_test(0xd53d000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_za, ggsext_fix_01){
  try {
    image_decode_test(0xdf54800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_in, ggsext_fix_01){
  try {
    image_decode_test(0xda40000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_ro, ggsext_fix_01){
  try {
    image_decode_test(0xe83a800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_ab, ggsext_fix_01){
  try {
    image_decode_test(0xec8d800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_or, ggsext_fix_01){
  try {
    image_decode_test(0xf19b000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_kl, ggsext_fix_01){
  try {
    image_decode_test(0xf6e0000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(ggsext_decode5_ju, ggsext_fix_01){
  try {
    image_decode_test(0xf985000);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

void sound_decode_test(u32 p_snd_base_ofs) {
  SoundDriver snd_drv;

  FILE *fp = fopen("data/SLPM_663.33/SCDDATA.BIN", "rb");
  if (fp) {
    int snd_num, ofs_table[1024];

    fseek(fp, p_snd_base_ofs, SEEK_SET);
    fread(&snd_num, sizeof(u32), 1, fp);

    fseek(fp, p_snd_base_ofs + 4, SEEK_SET);
    fread(ofs_table, sizeof(u32), snd_num, fp);

    int header_size = ((snd_num - 2) * 4 + 63) & ~63;

    for (int i = 0; i < snd_num - 1; i++) {
      int size = ofs_table[i + 1] - ofs_table[i];
      fseek(fp, p_snd_base_ofs + header_size + ofs_table[i], SEEK_SET);
      u8* buf = (u8*)malloc(size);
      fread(buf, 1, size, fp);

      int dstsize = snd_drv.get_adpcm_decoded_size(size);
      u8* dstbuf = (u8*)malloc(dstsize);
      int actual_size = snd_drv.decode_adpcm(dstbuf, buf, NULL, size);
      WIN_ASSERT_TRUE(actual_size > 0);
#if 0
      char str[256];
      sprintf(str, "memo/out%02d.wav", i);
      FILE* outfp = fopen(str, "wb");
      fwrite(dstbuf, 1, actual_size, outfp);
      fclose(outfp);
#endif
      free(dstbuf);
      free(buf);
    }
    fclose(fp);
  }
}

BEGIN_TESTF(ggsext_decode_sound_vo_so, ggsext_fix_01){
  try {
    // so vo = 0x183bd800
    // ky vo = 0x184e7000
    // ky se = 0x0a206010
    sound_decode_test(0x183bd800);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
