/*---------------------------------------------------------------*/
/*                            ds_util.h                          */
/*---------------------------------------------------------------*/
/*!
*  @file  ds_util.h
*  @author  ds
*  @brief  ���[�e�B���e�B�֐��Q
*/

#pragma once

/*-----*/
/* lib */
/*-----*/
#pragma comment(lib,"iphlpapi.lib")
#pragma comment(lib, "winmm.lib")

/*---------*/
/* include */
/*---------*/
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <string>
#include <vector>

/*---------*/
/* define  */
/*---------*/
#define IS_MB_CHAR(x)  (((unsigned char)x > 0x80 && (unsigned char)x < 0xa0) || ((unsigned char)x > 0xdf && (unsigned char)x < 0xfd))
#define IS_ASCII(x)    ((x) >= 0x20 && (x) < 0x7f)

#define FIXED2FLOAT(x)  ((float)x.value + (float)x.fract / 65536)
#define ARGB2XBGR(x)  ((x&0x0000ff00) | ((x&0xff)<<16) | ((x&0xff0000)>>16))

/*----------*/
/* function */
/*----------*/
/*!  @brief ���[�e�B���e�B�֐��Q
*/
namespace ds_util
{
  /*!  @brief  ���݉�����Ă���L�[���擾����
  *
  *  @param  int p_vk    ���ׂ�L�[
  *  @retval  bool      ������Ă�����true. �����łȂ����false.
  */
  bool isPressKey(int p_vk);
  bool isDownKey(int p_vk);

  /*!  @brief  NIC��MAC�A�h���X���擾����
  *
        p_buf�ɂ�6byte�ȏ�̃o�b�t�@���w�肷�邱��
  *  @param  int p_idx    NIC�̃C���f�b�N�X
  *  @param  char* p_buf    MAC�A�h���X(6byte)
  *  @retval  <none>
  */
  void  getMacAddress(int p_idx, char* p_buf);

  /*!  @brief  �V�X�e���f�B�X�N�̃V���A���i���o�[���擾����
  *
  *      �f�B�X�N�̃V���A���i���o�[�̓t�H�[�}�b�g�ŕύX�����
  *  @param  <none>
  *  @retval  DWORD      �V���A���i���o�[
  */
  DWORD getSysDiskSN(void);

  /*!  @brief  p_ptr����p_size���̃f�[�^��16�i���ŁA������ɕϊ�����
  *
  *  @param  char* p_ptr    �ϊ��Ώۃf�[�^�|�C���^
  *  @param  int p_size    �ϊ��Ώۃf�[�^�T�C�Y
  *  @param  char* p_str    �ϊ���̕�����o�b�t�@
  *  @retval  <none>
  */
  void hex2str(const char* p_ptr, int p_size, char* p_str);

  /*!  @brief  �������16�i���Ƃ݂Ȃ��l�ɕϊ��Ap_ptr����p_size���̗̈�Ɋi�[����
  *
  *  @param  char* p_ptr    �ϊ���̏������ݐ�f�[�^�|�C���^
  *  @param  int p_size    �ϊ���̏������ݐ�f�[�^�T�C�Y
  *  @param  char* p_str    �ϊ��Ώۂ̕�����o�b�t�@
  *  @retval  int        ���ۂɏ������񂾃f�[�^�T�C�Y
  */
  int str2hex(char* p_ptr, int p_size, const char* p_str, bool p_rev = false);

  char* strmid(char* p_dst, char* p_src, int p_st, int p_size);

  void strtrim(char* p_str);
  void strtrimL(char* p_str);
  void strtrimR(char* p_str);

  bool strmatch(const char* p_str, const char* p_pat);

  /*!  @brief  ����������ׂď������ɕϊ�����
  *
  *  @param  char* p_str    ������
  *  @retval  <none>
  */
  void strlower(char* p_str);

  /*!  @brief  ������p_s1��p_c1�̕����ŋ�؂�Bp_c2�ň͂܂�Ă���͈͂͋�؂�Ȃ��B
  *      �߂�l�ɒ��o�����������Ԃ�Ap_s1�͂��̎��̕�����̐擪�ɍX�V�����B
  *
  *  @param  char*&  p_s1  ������
  *      char  p_c1  ��؂蕶��
  *      char  p_c2  ���O����
  *  @retval  char*      ���o������
  */
  char* strdiv(char*& p_s1, char p_c1, char p_c2);

  /*!  @brief  �͈͎w�蕶��p_ch�ň͂܂ꂽ�����񂩂璆�g�̕�����𒊏o����
  *      �͈͎w�蕶�����A�����Ă����ꍇ�͒ʏ�̕����Ƃ��Ĉ����B
  *
  *  @param  char*    p_dst  �o�͕�����
  *  @param  int      p_size  �o�͕�����T�C�Y
  *  @param  const char* p_src  �Ώە�����
  *  @param  const char* p_ch  ����������
  *  @param  const char* p_ch  �u��������
  *  @retval  int          �t�@�C���T�C�Y
  */
  char* strreplace(char* p_dst, int p_dstsz, const char* p_src, int p_srcsz, const char* p_ch1, const char* p_ch2);

  /*!  @brief  �t�@�C���T�C�Y��32bit�Ŏ擾����
  *
  *  @param  FILE* p_fp    �t�@�C��
  *  @retval  int        �t�@�C���T�C�Y
  */
  int  fsize32(FILE* p_fp);

  /*!  @brief  �t�@�C���|�C���^�̈ʒu��32bit�Ŏ擾����
  *
  *  @param  FILE* p_fp    �t�@�C��
  *  @retval  int        �t�@�C���̈ʒu
  */
  int  fgetpos32(FILE* p_fp);

  /*!  @brief  �^����ꂽ�p�X�̐e�f�B���N�g�����擾����
  *
  *      ��؂��'/', '\'���Ή�
  *  @note  �@�\�I�ɒ��r���[�����ۂ߂Ȃ��̂�
  *      ��֋@�\���o������폜����邩��
  *  @param  char* p_path  ���̃p�X
  *  @param  char* p_out    �e�f�B���N�g���̃p�X
  *  @retval  <none>
  */
  void getParentDirectory(const char* p_path, char* p_out);

  /*!  @brief  �^����ꂽ�p�X�̃t�@�C�������擾����
  *
  *      ��؂��'/', '\'���Ή�
  *  @param  char* p_path  ���̃p�X
  *  @param  char* p_out    �t�@�C����
  *  @retval  <none>
  */
  void getFileName(const char* p_path, char* p_out);

  /*!  @brief  p_path1����p_path2�ւ̑��΃p�X�����߁Ap_out�ɕԂ�
  *
  *      ��؂��'/', '\'���Ή�
  *  @param  char* p_path    ���̃p�X
  *  @param  char* p_out      �t�@�C����
  *  @param  bool  p_matchcase  �p���̑啶������������ʂ���
  *  @retval  <none>
  */
  void getRelativePath(const char* p_path1, const char* p_path2, char* p_out, bool p_matchcase = false);

  std::string getFullPath(const std::string& p_path1,
                          const std::string& p_path2,
                          bool p_matchcase);

  // -----------------------------------------------------------------------------
  // �֐��� split_string
  // �T�v   p_delimiter�ŋ�؂�ꂽ�������z��ɕ�������
  // ����   const std::string&        p_in        ���͕�����
  //        std::vector<std::string>* p_out       �o�͕�����z��|�C���^
  //        std::string               p_delimiter ��؂蕶����
  //        bool                      p_shrink    �󕶎���j�����ċl�߂邩�H
  // �߂�l �Ȃ�
  void split_string(const std::string& p_in,
                    std::vector<std::string>* p_out,
                    std::string p_delimiter, bool p_shrink);

  // -----------------------------------------------------------------------------
  // �֐��� join_string
  // �T�v   ������z���p_wrap�ň͂�p_delimiter�ŋ�؂��ꂽ������֌�������
  // ����   const std::vector<std::string>& p_in        ���͕�����z��
  //        s8                              p_delimiter ��؂蕶��
  //        s8                              p_wrap1     �O���͂�����
  //        s8                              p_wrap2     ����͂�����
  // �߂�l std::string �����ϕ�����
  std::string join_string(const std::vector<std::string>& p_in,
                          char p_delimiter, char p_wrap1, char p_wrap2);

  /*!  @brief  ������𐔒l�ɕϊ�����
  *
  *      <p_radix>�i���Ƃ��ĕ�����𐔒l�ɕϊ�����
  *  @param  char* p_str    �Ώە�����
  *  @param  int p_radix    �
  *  @retval  <none>
  */
  DWORD atox(char* p_str, int p_radix);

//=========================================================
// RECT�֘A
//=========================================================

  /*!  @brief  ��`�̕����擾����
  *
  *  @param  LPRECT  p_rect  �Ώۋ�`
  *  @retval  int        ��`�̕�
  */
  inline int getRectWidth(LPRECT p_rect) { return p_rect->right - p_rect->left; }

  /*!  @brief  ��`�̍������擾����
  *
  *  @param  LPRECT  p_rect  �Ώۋ�`
  *  @retval  int        ��`�̍���
  */
  inline int getRectHeight(LPRECT p_rect) { return p_rect->bottom - p_rect->top; }

  /*!  @brief  ��`���ʐςO�̏ꍇtrue��Ԃ�
  *
  *  @param  LPRECT  p_rect  �Ώۋ�`
  *  @retval  bool      ��`���ʐςO���ǂ���
  */
  inline bool isEmptyRect(LPRECT p_rect) { return (p_rect->right == p_rect->left) || (p_rect->bottom == p_rect->top); }

  /*!  @brief  left/right��top/bottom�����]���Ă����`���C������
  *
  *  @param  LPRECT  p_rect  �Ώۋ�`
  *  @retval  <none>
  */
  void normalizeRect(LPRECT p_rect);

//=========================================================
// ������͊֘A
//=========================================================
  int  createArgString(char* p_cmdline, char**& p_argv);
  void deleteArgString(int p_argc, char**& p_argv);

  /*!  @brief  ��������͂��A�����̔z��C���f�b�N�X��Ԃ�
  *
  *  @param  int argc    ��͑Ώ۔z��v�f���i���s�t�@�C���ɓn���ꂽ����argc�����̂܂ܓn���΂悢�j
  *  @param  char** p_argv  ��͑Ώ۔z��i���s�t�@�C���ɓn���ꂽ����argv�����̂܂ܓn���΂悢�j
  *  @param  char* p_str    ��������
  *  @retval  int        ���������̔z��C���f�b�N�X�i���݂��Ȃ��ꍇ�� -1 ���ԋp�����j
  */
  int findArg(int p_argc, char** p_argv, char* p_str);
  
  /*!  @brief  ��������͂��A���̒l�̔z��C���f�b�N�X��Ԃ�
  *
  *  @param  int argc    ��͑Ώ۔z��v�f���i���s�t�@�C���ɓn���ꂽ����argc�����̂܂ܓn���΂悢�j
  *  @param  char** p_argv  ��͑Ώ۔z��i���s�t�@�C���ɓn���ꂽ����argv�����̂܂ܓn���΂悢�j
  *  @param  char* p_str    ����������
  *  @param  int p_idx    �擾����l�̃C���f�b�N�X
  *  @retval  int        ����������̒l�̔z��C���f�b�N�X�i���݂��Ȃ��ꍇ�� -1 ���ԋp�����j
  */
  int findArgValue(int p_argc, char** p_argv, char* p_str, int p_valueIdx = 0);

//=========================================================
// GDI�`��֘A
//=========================================================
  /*!  @brief  �F�w��\�ȕ�����`��
  *
  *  '@@'��Ɉȉ��̎��ʎq���w��\  <BR>
  *  <TABLE>
  *    <TR><TD>����</TD><TD>�J���[�l</TD></TR>
  *    <TR><TD>x</TD><TD>  0,  0,  0</TD></TR>
  *    <TR><TD>d</TD><TD>128,128,128</TD></TR>
  *    <TR><TD>w</TD><TD>255,255,255</TD></TR>
  *    <TR><TD>r</TD><TD>255,  0,  0</TD></TR>
  *    <TR><TD>g</TD><TD>  0,255,  0</TD></TR>
  *    <TR><TD>b</TD><TD>  0,  0,255</TD></TR>
  *    <TR><TD>c</TD><TD>  0,255,255</TD></TR>
  *    <TR><TD>y</TD><TD>255,  0,255</TD></TR>
  *    <TR><TD>m</TD><TD>255,255,  0</TD></TR>
  *    <TR><TD>R</TD><TD>128,  0,  0</TD></TR>
  *    <TR><TD>G</TD><TD>  0,128,  0</TD></TR>
  *    <TR><TD>B</TD><TD>  0,  0,128</TD></TR>
  *  </TABLE>
  *  @param  HDC p_hdc      �f�o�C�X�R���e�L�X�g�n���h��
  *  @param  int p_x        �`��J�n�ʒux
  *  @param  int p_y        �`��J�n�ʒuy
  *  @param  char* p_str      �`�敶����
  *  @param  COLORREF p_color  �f�t�H���g�J���[
  *  @retval  <none>
  *  \n
  *  @code
  *  // �T���v���R�[�h
  *  TextOutColor(hdc, 100, 100, "@rRED@gGREEN@bBLUE", 0x00000000);
  *
  *  @endcode
  */
  void TextOutColor(HDC p_hdc, int p_x, int p_y, char* p_str, COLORREF p_color);

//=========================================================
// �t���[�����[�g�֘A
//=========================================================
  /*!  @brief  �t���[�����[�g���擾����
  *
  *      �O����s������̎��ԍ�������t���[�����[�g�l���Z�o����
  *  @param  <none>
  *  @retval  int    �t���[�����[�g
  */
  int getfps(void);

  /*!  @brief  �`��Ԋu��60fps�ɂȂ�悤�ɑҋ@����
  *
  *      �O����s������̎��ԍ�����16.6...(ms)�ɂȂ�܂őҋ@����
  *  @param  <none>
  *  @retval  <none>
  */
  void sync60fps(void);

//=========================================================
// �p�t�H�[�}���X�J�E���^�֘A
//=========================================================
  /*!  @brief  ���A���^�C���N���b�N���擾����
  *  
  *  @param  <none>
  *  @retval  __int64    ���A���^�C���N���b�N�l
  */
  inline __int64 getRTSC(void)
  {
    __int64 time;
    __asm
    {
      rdtsc
      mov dword ptr [time], eax
      mov dword ptr [time + 4], edx
    }
    return time;
  }

  /*!  @brief  �~���b�P�ʂŎ��Ԃ��擾����
  *  
  *  @param  <none>
  *  @retval  DWORD    �V�X�e�����N������̌o�ߎ��Ԓl
  */
  inline DWORD getTime(void) { return timeGetTime(); }

//=========================================================
// �����������Z�R���g���[�����[�h�֘A
//=========================================================
  /*!  @brief  �����������Z�R���g���[�����[�h�l���擾����
  *  
  *  @param  <none>
  *  @retval  WORD    �����������Z�R���g���[�����[�h�l
  */
  WORD getcw(void);

  /*!  @brief  �����������Z�R���g���[�����[�h�l��ݒ肷��
  *  
  *  @param  WORD p_cw  �����������Z�R���g���[�����[�h�l
  *  @retval  <none>
  */
  void setcw(WORD p_cw);

//=========================================================
// �F��ԕϊ��֘A
//=========================================================
  /*!  @brief  RGB����HSB�ɕϊ�����
  *
  *  @param  int p_r      ����Red
  *  @param  int p_g      ����Green
  *  @param  int p_b      ����Blue
  *  @param  float &p_hue  �o��Hue
  *  @param  float &p_sat  �o��Saturation
  *  @param  float &p_bri  �o��Brightness
  *  @retval  <none>
  */
  void RGB2HSB(int p_r, int p_g, int p_b, float &p_hue, float &p_sat, float &p_bri);

  /*!  @brief  HSB����RGB�ɕϊ�����
  *
  *  @param  float &p_hue  ����Hue
  *  @param  float &p_sat  ����Saturation
  *  @param  float &p_bri  ����Brightness
  *  @param  int p_r      �o��Red
  *  @param  int p_g      �o��Green
  *  @param  int p_b      �o��Blue
  *  @retval  <none>
  */
  void HSB2RGB(float p_hue, float p_sat, float p_bri, int &p_r, int &p_g, int &p_b);

  /*!  @brief  RGB����YUV�ɕϊ�����
  *
  *  @param  int p_r      ����Red
  *  @param  int p_g      ����Green
  *  @param  int p_b      ����Blue
  *  @param  float &p_hue  �o��Y
  *  @param  float &p_sat  �o��U
  *  @param  float &p_bri  �o��V
  *  @retval  <none>
  */
  void rgb2yuvf(const double p_r, const double p_g, const double p_b, double &p_y, double &p_u, double &p_v);
  void rgb2yuv(const long p_r, const long p_g, const long p_b, long &p_y, long &p_u, long &p_v);

  /*!  @brief  YUV����RGB�ɕϊ�����
  *
  *  @param  double &p_hue  ����Y
  *  @param  double &p_sat  ����U
  *  @param  double &p_bri  ����V
  *  @param  int p_r      �o��Red
  *  @param  int p_g      �o��Green
  *  @param  int p_b      �o��Blue
  *  @retval  <none>
  */
  void yuv2rgbf(const double p_y, const double p_u, const double p_v, double &p_r, double &p_g, double &p_b);
  void yuv2rgb(const long p_y, const long p_u, const long p_v, long &p_r, long &p_g, long &p_b);

  void rgb2ycbcr(const long p_r, const long p_g, const long p_b, long &p_y, long &p_u, long &p_v);
  void ycbcr2rgb(const long p_y, const long p_u, const long p_v, long &p_r, long &p_g, long &p_b);

//=========================================================
// �ėp�A���S���Y���֘A
//=========================================================
  /*!  @brief  �K�E�X�E�W�����_���@�ɂ��A��������������
  *
  *  @param  double* m    ���͍s��
  *  @param  int p_row    �s��
  *  @param  int o_col    ��
  *  @retval  <none>
  */
  void gauss_jordan(double* m, int p_row, int p_col);

  /*!  @brief  �R���X�v���C�����
  *
  *  @retval  <none>
  */
  double spline(double* p_xa, double* p_ya, int p_len, double p_x);

#ifdef _DEBUG
  void DBGOUTA(char* fmt, ...);
  void DBGOUTW(wchar_t* fmt, ...);
#else
  inline void DBGOUTA(char* fmt, ...) {}
  inline void DBGOUTW(wchar_t* fmt, ...) {}
#endif
};
