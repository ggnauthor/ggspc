#ifndef _debugger_gui_memorywindow
#define _debugger_gui_memorywindow

/*---------*/
/* include */
/*---------*/
#include "debugger/gui/list_window_base.h"

/*--------*/
/* define */
/*--------*/

/*-------*/
/* class */
/*-------*/
class MemoryWindow : public ListWindowBase
{
// constant definition
public:
  enum {
    kCtrlPnlHeight  = 22,
    kLabelHeight  = 16,
#ifdef _ENV2
    kViewX = 320 + 150,
    kViewY = 670 + 40,
    kViewW = 400,
    kViewH = 150,
#else
    kViewX = 558,
    kViewY = 705,
    kViewW = 426,
    kViewH = 195,
#endif
  };

// constructor/destructor
public:
  MemoryWindow();
  ~MemoryWindow();

// override
public:
  virtual void Init();
  virtual bool PreTranslateMessage(MSG* p_msg);
  virtual void Draw();

private:
  virtual int  GetListHeaderTop() { return kCtrlPnlHeight; }
  virtual int  GetListTop() { return kCtrlPnlHeight + kLabelHeight; }
  virtual int  GetAllLine();
  //virtual int  GetPageLine();
  //virtual void InitScroll();
  virtual void SetScrollMax(int p_line_count);
  //virtual void SortListData(QSORT_FUNC p_sort_func);

// event handler
public:
  void OnInitDialog(HWND p_hwnd);
  //void OnActivate(int p_active, int p_minimized, int p_hwnd_pre);
  void OnCommand(int p_ctrlID, int p_notify);
  void OnShowWindow(WPARAM p_wp, LPARAM p_lp);
  //void OnSize(WPARAM p_wp, LPARAM p_lp);
  //void OnVScroll(WPARAM p_wp, LPARAM p_lp);
  //void OnPaint(WPARAM p_wp, LPARAM p_lp);
  //void OnMouseWheel(WPARAM p_wp, LPARAM p_lp);
  //void OnMouseMove(int p_x, int p_y);
  void OnLButtonDown(int p_x, int p_y);
  void OnLButtonDblClk(int p_x, int p_y);
  //void OnLButtonUp(int p_x, int p_y);
  void OnRButtonDown(int p_x, int p_y);
  void OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift);
  //void OnDrawItem(int p_ctrlID, DRAWITEMSTRUCT* p_di);
  //void OnMenuSelect(int p_menu_id, int p_menu_flag, HMENU p_menu_handle);

  //void OnClear();

// member function
public:
  void ShowCurrentAddress();
  void SelectAddress(u32 p_va, bool p_redraw = true);
  void OpenMemoryEditDialog();
  void OpenDumpDialog();
  void OpenFindBinaryDialog();

// accessor
  inline bool jump_ref_addr() { return m_jump_ref_addr; }

// member variable
private:
  int    m_offset;      // 
  bool  m_jump_ref_addr;  // 命令が参照したメモリに自動でスクロールインする
  bool  m_edit_low4bits;  // 次の入力で下位4ビットを編集する
};

#endif // _debugger_gui_memorywindow
