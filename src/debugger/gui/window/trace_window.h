#ifndef _debugger_gui_callstackwindow
#define _debugger_gui_callstackwindow

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
class TraceWindow : public ListWindowBase
{
// constant definition
public:
  enum {
    kCtrlPnlHeight  = 22,
    kLabelHeight = 16,
    
#ifdef _ENV2
    kViewX = 10  + 150,
    kViewY = 700 + 40,
    kViewW = 304,
    kViewH = 120,
#else
    kViewX = 376,
    kViewY = 50,
    kViewW = 174,
    kViewH = 260,
#endif
  };

  enum {
    kCat_CallStack = 0,
    kCat_JumpLog,
  };

  enum {
    kJumpLog_Size = 256
  };

// constructor/destructor
public:
  TraceWindow();
  ~TraceWindow();

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
  void Draw_CallStack(HDC p_hdc);
  void Draw_JumpLog(HDC p_hdc);

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
  //void OnLButtonDown(int p_x, int p_y);
  //void OnLButtonDblClk(int p_x, int p_y);
  //void OnLButtonUp(int p_x, int p_y);
  //void OnRButtonDown(int p_x, int p_y);
  void OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift);
  //void OnDrawItem(int p_ctrlID, DRAWITEMSTRUCT* p_di);
  //void OnMenuSelect(int p_menu_id, int p_menu_flag, HMENU p_menu_handle);

  //void OnClear();

// member variable
private:
  int    m_category;
  HFONT  m_small_font;
};

#endif // _debugger_gui_callstackwindow
