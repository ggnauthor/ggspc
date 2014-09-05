#ifndef _debugger_gui_codewindow
#define _debugger_gui_codewindow

#include "debugger/gui/list_window_base.h"

class ModuleInfo;

class CodeWindow : public ListWindowBase {
// constant definition
public:
  enum {
    kCtrlPnlHeight  = 22,
    kLabelHeight  = 16,
#ifdef _ENV2
    kViewX = 320 + 150,
    kViewY = 70  + 40,
    kViewW = 400,
    kViewH = 580,
#else
    kViewX = 50,
    kViewY = 334,
    kViewW = 500,
    kViewH = 566,
#endif
  };

// constructor/destructor
public:
  CodeWindow();
  ~CodeWindow();

// override
public:
  virtual void Init();
  virtual bool PreTranslateMessage(MSG* p_msg);
  virtual void Draw();

protected:
  virtual int  GetListHeaderTop() { return kCtrlPnlHeight; }
  virtual int  GetListTop() { return kCtrlPnlHeight + kLabelHeight; }
  virtual int  GetAllLine();
  //virtual int  GetPageLine();
  //virtual void InitScroll();
  //virtual void SetScrollMax(int p_line_count);
  //virtual void SortListData(QSORT_FUNC p_sort_func);

private:
  void Draw_Assembly(HDC p_hdc);
  void Draw_Source(HDC p_hdc);

// event handler
public:
  //void OnInitDialog(HWND p_hwnd);
  //void OnActivate(int p_active, int p_minimized, int p_hwnd_pre);
  void OnCommand(int p_ctrlID, int p_notify);
  void OnShowWindow(WPARAM p_wp, LPARAM p_lp);
  //void OnSize(WPARAM p_wp, LPARAM p_lp);
  //void OnVScroll(WPARAM p_wp, LPARAM p_lp);
  //void OnPaint(WPARAM p_wp, LPARAM p_lp);
  //void OnMouseWheel(WPARAM p_wp, LPARAM p_lp);
  //void OnMouseMove(int p_x, int p_y);
  void OnLButtonDown(int p_x, int p_y);
  //void OnLButtonDblClk(int p_x, int p_y);
  //void OnLButtonUp(int p_x, int p_y);
  void OnRButtonDown(int p_x, int p_y);
  void OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift);
  void OnDrawItem(int p_ctrlID, DRAWITEMSTRUCT* p_di);
  void OnMenuSelect(int p_menu_id, int p_menu_flag, HMENU p_menu_handle);

  //void OnClear();

// member function
public:
  void ModuleSetup();
  ModuleInfo* GetCurModule();
  void AutoJumpModule(u32 p_va);

  int  VA2Line(u32 p_va);
  void SelectAddress(u32 p_va, bool p_redraw = true);
  bool TranslateAccelerator(MSG* p_msg);

  bool HasSource();
  bool HasAssembler(int p_line);

private:
  bool OpenLineDialog();
  bool OpenAddrJumpDialog();
  bool OpenFindInstDialog();
  bool OpenBreakInfoDialog(int p_idx);

  void handleDebugExecute(int p_ctrlID);
  void handleDebugPause();
  void handleDebugToggleBreak();
  void handleDebugBreakDialog();
  void handleDebugTogglePatch();
  void handleDebugSetPC();
  void handleDebugEdit();
  void handleDisplayGoDialog();
  void handleDisplayGoPC();
  void handleDisplayGoRef();
  void handleDisplayGoPrev();
  void handleDisplaySource();
  void handleDisplayAssembly();
  void handleDisplayFindRef();
  void handleDisplayFindInst();
  void handleProfilerEnable(int p_ctrlID);
  void handleProfilerSetStart();
  void handleProfilerSetEnd();
  void handleChangeSource();
  void handleChangeMemory();

  void drawAssemblyOpcodeCol(
    HDC p_hdc, int p_line, int p_left, bool p_is_analyzed);
  void drawAssemblyMacroRecIdCol(
    HDC p_hdc, int p_line, int p_left);
  void drawAssemblyCodeCol(
    HDC p_hdc, int p_line, int p_left, bool p_is_analyzed);
  void drawAssemblyCommentCol(
    HDC p_hdc, int p_line, int p_left);

  void drawSourceLineCol(
    HDC p_hdc, int p_line, int p_left);
  void drawSourceCodeCol(
    HDC p_hdc, int p_line, int p_left,
    bool* p_c_comment, bool* p_cpp_comment, bool* p_in_string);

private:
  // モジュール選択コンボの状態
  int module_idx_;
  // ソース選択コンボの状態
  int src_idx_;
  // アクセラレータ
  HACCEL accel_;
  // ←→キーによるジャンプの移動履歴
  vector<u32> browse_log_;

  static const s8 kText_ColStr_Addr[];
  static const s8 kText_ColStr_Opcode[];
  static const s8 kText_ColStr_MacroRecompile[];
  static const s8 kText_ColStr_Assembly[];
  static const s8 kText_ColStr_Comment[];
  static const s8 kText_ColStr_Line[];
  static const s8 kText_ColStr_Source[];
};

#endif // _debugger_gui_codewindow
