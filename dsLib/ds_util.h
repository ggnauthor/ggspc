/*---------------------------------------------------------------*/
/*                            ds_util.h                          */
/*---------------------------------------------------------------*/
/*!
*  @file  ds_util.h
*  @author  ds
*  @brief  ユーティリティ関数群
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
/*!  @brief ユーティリティ関数群
*/
namespace ds_util
{
  /*!  @brief  現在押されているキーを取得する
  *
  *  @param  int p_vk    調べるキー
  *  @retval  bool      押されていたらtrue. そうでなければfalse.
  */
  bool isPressKey(int p_vk);
  bool isDownKey(int p_vk);

  /*!  @brief  NICのMACアドレスを取得する
  *
        p_bufには6byte以上のバッファを指定すること
  *  @param  int p_idx    NICのインデックス
  *  @param  char* p_buf    MACアドレス(6byte)
  *  @retval  <none>
  */
  void  getMacAddress(int p_idx, char* p_buf);

  /*!  @brief  システムディスクのシリアルナンバーを取得する
  *
  *      ディスクのシリアルナンバーはフォーマットで変更される
  *  @param  <none>
  *  @retval  DWORD      シリアルナンバー
  */
  DWORD getSysDiskSN(void);

  /*!  @brief  p_ptrからp_size分のデータを16進数で、文字列に変換する
  *
  *  @param  char* p_ptr    変換対象データポインタ
  *  @param  int p_size    変換対象データサイズ
  *  @param  char* p_str    変換後の文字列バッファ
  *  @retval  <none>
  */
  void hex2str(const char* p_ptr, int p_size, char* p_str);

  /*!  @brief  文字列を16進数とみなし値に変換、p_ptrからp_size分の領域に格納する
  *
  *  @param  char* p_ptr    変換後の書き込み先データポインタ
  *  @param  int p_size    変換後の書き込み先データサイズ
  *  @param  char* p_str    変換対象の文字列バッファ
  *  @retval  int        実際に書き込んだデータサイズ
  */
  int str2hex(char* p_ptr, int p_size, const char* p_str, bool p_rev = false);

  char* strmid(char* p_dst, char* p_src, int p_st, int p_size);

  void strtrim(char* p_str);
  void strtrimL(char* p_str);
  void strtrimR(char* p_str);

  bool strmatch(const char* p_str, const char* p_pat);

  /*!  @brief  文字列をすべて小文字に変換する
  *
  *  @param  char* p_str    文字列
  *  @retval  <none>
  */
  void strlower(char* p_str);

  /*!  @brief  文字列p_s1をp_c1の文字で区切る。p_c2で囲まれている範囲は区切らない。
  *      戻り値に抽出した文字が返り、p_s1はその次の文字列の先頭に更新される。
  *
  *  @param  char*&  p_s1  文字列
  *      char  p_c1  区切り文字
  *      char  p_c2  除外文字
  *  @retval  char*      抽出文字列
  */
  char* strdiv(char*& p_s1, char p_c1, char p_c2);

  /*!  @brief  範囲指定文字p_chで囲まれた文字列から中身の文字列を抽出する
  *      範囲指定文字が連続していた場合は通常の文字として扱う。
  *
  *  @param  char*    p_dst  出力文字列
  *  @param  int      p_size  出力文字列サイズ
  *  @param  const char* p_src  対象文字列
  *  @param  const char* p_ch  検索文字列
  *  @param  const char* p_ch  置換文字列
  *  @retval  int          ファイルサイズ
  */
  char* strreplace(char* p_dst, int p_dstsz, const char* p_src, int p_srcsz, const char* p_ch1, const char* p_ch2);

  /*!  @brief  ファイルサイズを32bitで取得する
  *
  *  @param  FILE* p_fp    ファイル
  *  @retval  int        ファイルサイズ
  */
  int  fsize32(FILE* p_fp);

  /*!  @brief  ファイルポインタの位置を32bitで取得する
  *
  *  @param  FILE* p_fp    ファイル
  *  @retval  int        ファイルの位置
  */
  int  fgetpos32(FILE* p_fp);

  /*!  @brief  与えられたパスの親ディレクトリを取得する
  *
  *      区切りは'/', '\'両対応
  *  @note  機能的に中途半端感が否めないので
  *      代替機能が出来次第削除されるかも
  *  @param  char* p_path  元のパス
  *  @param  char* p_out    親ディレクトリのパス
  *  @retval  <none>
  */
  void getParentDirectory(const char* p_path, char* p_out);

  /*!  @brief  与えられたパスのファイル名を取得する
  *
  *      区切りは'/', '\'両対応
  *  @param  char* p_path  元のパス
  *  @param  char* p_out    ファイル名
  *  @retval  <none>
  */
  void getFileName(const char* p_path, char* p_out);

  /*!  @brief  p_path1からp_path2への相対パスを求め、p_outに返す
  *
  *      区切りは'/', '\'両対応
  *  @param  char* p_path    元のパス
  *  @param  char* p_out      ファイル名
  *  @param  bool  p_matchcase  英字の大文字小文字を区別する
  *  @retval  <none>
  */
  void getRelativePath(const char* p_path1, const char* p_path2, char* p_out, bool p_matchcase = false);

  std::string getFullPath(const std::string& p_path1,
                          const std::string& p_path2,
                          bool p_matchcase);

  // -----------------------------------------------------------------------------
  // 関数名 split_string
  // 概要   p_delimiterで区切られた文字列を配列に分割する
  // 引数   const std::string&        p_in        入力文字列
  //        std::vector<std::string>* p_out       出力文字列配列ポインタ
  //        std::string               p_delimiter 区切り文字列
  //        bool                      p_shrink    空文字を破棄して詰めるか？
  // 戻り値 なし
  void split_string(const std::string& p_in,
                    std::vector<std::string>* p_out,
                    std::string p_delimiter, bool p_shrink);

  // -----------------------------------------------------------------------------
  // 関数名 join_string
  // 概要   文字列配列をp_wrapで囲いp_delimiterで区切りられた文字列へ結合する
  // 引数   const std::vector<std::string>& p_in        入力文字列配列
  //        s8                              p_delimiter 区切り文字
  //        s8                              p_wrap1     前方囲い文字
  //        s8                              p_wrap2     後方囲い文字
  // 戻り値 std::string 結合済文字列
  std::string join_string(const std::vector<std::string>& p_in,
                          char p_delimiter, char p_wrap1, char p_wrap2);

  /*!  @brief  文字列を数値に変換する
  *
  *      <p_radix>進数として文字列を数値に変換する
  *  @param  char* p_str    対象文字列
  *  @param  int p_radix    基数
  *  @retval  <none>
  */
  DWORD atox(char* p_str, int p_radix);

//=========================================================
// RECT関連
//=========================================================

  /*!  @brief  矩形の幅を取得する
  *
  *  @param  LPRECT  p_rect  対象矩形
  *  @retval  int        矩形の幅
  */
  inline int getRectWidth(LPRECT p_rect) { return p_rect->right - p_rect->left; }

  /*!  @brief  矩形の高さを取得する
  *
  *  @param  LPRECT  p_rect  対象矩形
  *  @retval  int        矩形の高さ
  */
  inline int getRectHeight(LPRECT p_rect) { return p_rect->bottom - p_rect->top; }

  /*!  @brief  矩形が面積０の場合trueを返す
  *
  *  @param  LPRECT  p_rect  対象矩形
  *  @retval  bool      矩形が面積０かどうか
  */
  inline bool isEmptyRect(LPRECT p_rect) { return (p_rect->right == p_rect->left) || (p_rect->bottom == p_rect->top); }

  /*!  @brief  left/rightやtop/bottomが反転している矩形を修正する
  *
  *  @param  LPRECT  p_rect  対象矩形
  *  @retval  <none>
  */
  void normalizeRect(LPRECT p_rect);

//=========================================================
// 引数解析関連
//=========================================================
  int  createArgString(char* p_cmdline, char**& p_argv);
  void deleteArgString(int p_argc, char**& p_argv);

  /*!  @brief  引数を解析し、引数の配列インデックスを返す
  *
  *  @param  int argc    解析対象配列要素数（実行ファイルに渡された引数argcをそのまま渡せばよい）
  *  @param  char** p_argv  解析対象配列（実行ファイルに渡された引数argvをそのまま渡せばよい）
  *  @param  char* p_str    検索引数
  *  @retval  int        検索引数の配列インデックス（存在しない場合は -1 が返却される）
  */
  int findArg(int p_argc, char** p_argv, char* p_str);
  
  /*!  @brief  引数を解析し、その値の配列インデックスを返す
  *
  *  @param  int argc    解析対象配列要素数（実行ファイルに渡された引数argcをそのまま渡せばよい）
  *  @param  char** p_argv  解析対象配列（実行ファイルに渡された引数argvをそのまま渡せばよい）
  *  @param  char* p_str    検索文字列
  *  @param  int p_idx    取得する値のインデックス
  *  @retval  int        検索文字列の値の配列インデックス（存在しない場合は -1 が返却される）
  */
  int findArgValue(int p_argc, char** p_argv, char* p_str, int p_valueIdx = 0);

//=========================================================
// GDI描画関連
//=========================================================
  /*!  @brief  色指定可能な文字列描画
  *
  *  '@@'後に以下の識別子を指定可能  <BR>
  *  <TABLE>
  *    <TR><TD>文字</TD><TD>カラー値</TD></TR>
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
  *  @param  HDC p_hdc      デバイスコンテキストハンドル
  *  @param  int p_x        描画開始位置x
  *  @param  int p_y        描画開始位置y
  *  @param  char* p_str      描画文字列
  *  @param  COLORREF p_color  デフォルトカラー
  *  @retval  <none>
  *  \n
  *  @code
  *  // サンプルコード
  *  TextOutColor(hdc, 100, 100, "@rRED@gGREEN@bBLUE", 0x00000000);
  *
  *  @endcode
  */
  void TextOutColor(HDC p_hdc, int p_x, int p_y, char* p_str, COLORREF p_color);

//=========================================================
// フレームレート関連
//=========================================================
  /*!  @brief  フレームレートを取得する
  *
  *      前回実行時からの時間差分からフレームレート値を算出する
  *  @param  <none>
  *  @retval  int    フレームレート
  */
  int getfps(void);

  /*!  @brief  描画間隔が60fpsになるように待機する
  *
  *      前回実行時からの時間差分が16.6...(ms)になるまで待機する
  *  @param  <none>
  *  @retval  <none>
  */
  void sync60fps(void);

//=========================================================
// パフォーマンスカウンタ関連
//=========================================================
  /*!  @brief  リアルタイムクロックを取得する
  *  
  *  @param  <none>
  *  @retval  __int64    リアルタイムクロック値
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

  /*!  @brief  ミリ秒単位で時間を取得する
  *  
  *  @param  <none>
  *  @retval  DWORD    システムが起動からの経過時間値
  */
  inline DWORD getTime(void) { return timeGetTime(); }

//=========================================================
// 浮動少数演算コントロールワード関連
//=========================================================
  /*!  @brief  浮動少数演算コントロールワード値を取得する
  *  
  *  @param  <none>
  *  @retval  WORD    浮動少数演算コントロールワード値
  */
  WORD getcw(void);

  /*!  @brief  浮動少数演算コントロールワード値を設定する
  *  
  *  @param  WORD p_cw  浮動少数演算コントロールワード値
  *  @retval  <none>
  */
  void setcw(WORD p_cw);

//=========================================================
// 色空間変換関連
//=========================================================
  /*!  @brief  RGBからHSBに変換する
  *
  *  @param  int p_r      入力Red
  *  @param  int p_g      入力Green
  *  @param  int p_b      入力Blue
  *  @param  float &p_hue  出力Hue
  *  @param  float &p_sat  出力Saturation
  *  @param  float &p_bri  出力Brightness
  *  @retval  <none>
  */
  void RGB2HSB(int p_r, int p_g, int p_b, float &p_hue, float &p_sat, float &p_bri);

  /*!  @brief  HSBからRGBに変換する
  *
  *  @param  float &p_hue  入力Hue
  *  @param  float &p_sat  入力Saturation
  *  @param  float &p_bri  入力Brightness
  *  @param  int p_r      出力Red
  *  @param  int p_g      出力Green
  *  @param  int p_b      出力Blue
  *  @retval  <none>
  */
  void HSB2RGB(float p_hue, float p_sat, float p_bri, int &p_r, int &p_g, int &p_b);

  /*!  @brief  RGBからYUVに変換する
  *
  *  @param  int p_r      入力Red
  *  @param  int p_g      入力Green
  *  @param  int p_b      入力Blue
  *  @param  float &p_hue  出力Y
  *  @param  float &p_sat  出力U
  *  @param  float &p_bri  出力V
  *  @retval  <none>
  */
  void rgb2yuvf(const double p_r, const double p_g, const double p_b, double &p_y, double &p_u, double &p_v);
  void rgb2yuv(const long p_r, const long p_g, const long p_b, long &p_y, long &p_u, long &p_v);

  /*!  @brief  YUVからRGBに変換する
  *
  *  @param  double &p_hue  入力Y
  *  @param  double &p_sat  入力U
  *  @param  double &p_bri  入力V
  *  @param  int p_r      出力Red
  *  @param  int p_g      出力Green
  *  @param  int p_b      出力Blue
  *  @retval  <none>
  */
  void yuv2rgbf(const double p_y, const double p_u, const double p_v, double &p_r, double &p_g, double &p_b);
  void yuv2rgb(const long p_y, const long p_u, const long p_v, long &p_r, long &p_g, long &p_b);

  void rgb2ycbcr(const long p_r, const long p_g, const long p_b, long &p_y, long &p_u, long &p_v);
  void ycbcr2rgb(const long p_y, const long p_u, const long p_v, long &p_r, long &p_g, long &p_b);

//=========================================================
// 汎用アルゴリズム関連
//=========================================================
  /*!  @brief  ガウス・ジョルダン法により連立方程式を解く
  *
  *  @param  double* m    入力行列
  *  @param  int p_row    行数
  *  @param  int o_col    列数
  *  @retval  <none>
  */
  void gauss_jordan(double* m, int p_row, int p_col);

  /*!  @brief  ３次スプライン補間
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
