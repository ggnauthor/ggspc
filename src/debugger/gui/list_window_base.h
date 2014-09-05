#ifndef _debugger_gui_listwindowbasex
#define _debugger_gui_listwindowbasex

#pragma comment(lib, "comctl32.lib")

#include "main.h"

//#define _WIN32_IE 0x0501
#include <commctrl.h>

#include <utils/ds_window.h>

/*--------*/
/* define */
/*--------*/

/*-------*/
/* class */
/*-------*/
struct ListWindowCol {
  enum {
    kSortAscend = 0,
    kSortDescend = 1,
    kSortNone = 2,
  };

  char label[256];
  int  width;
  bool fix;
  int  sort_type;
  QSORT_FUNC (*sort_func)(bool);
};

class ListWindowBase {
// constant definition
public:
  enum {
    kFontSize = 12,
  };

// constructor/destructor
public:
  ListWindowBase();
  virtual ~ListWindowBase();

// override
public:
  virtual void Init(int p_res_id, DLGPROC p_dlg_proc, int p_x, int p_y, int p_w, int p_h);
  virtual bool PreTranslateMessage(MSG* p_msg) = 0;
  virtual void Draw() = 0;
  
protected:
  void clearColumns();
  virtual int  GetListHeaderTop() = 0;
  virtual int  GetListTop() = 0;
  virtual int  GetAllLine() = 0;
  virtual int  GetPageLine();
  virtual void InitScroll();
  virtual void SetScrollMax(int p_line_count);
  virtual void SortListData(QSORT_FUNC p_sort_func) {}

// event handler
public:
  virtual void OnInitDialog(HWND p_hwnd) {}
  virtual void OnActivate(int p_active, int p_minimized, int p_hwnd_pre) {}
  virtual void OnCommand(int p_ctrlID, int p_notify) {}
  virtual void OnShowWindow(WPARAM p_wp, LPARAM p_lp) {}
  virtual void OnSize(WPARAM p_wp, LPARAM p_lp);
  virtual void OnVScroll(WPARAM p_wp, LPARAM p_lp);
  virtual void OnPaint(WPARAM p_wp, LPARAM p_lp) { Draw(); }
  virtual void OnMouseWheel(WPARAM p_wp, LPARAM p_lp);
  virtual void OnMouseMove(int p_x, int p_y);
  virtual void OnLButtonDown(int p_x, int p_y);
  virtual void OnLButtonDblClk(int p_x, int p_y);
  virtual void OnLButtonUp(int p_x, int p_y);
  virtual void OnRButtonDown(int p_x, int p_y) {}
  virtual void OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift) {}
  virtual void OnDrawItem(int p_ctrlID, DRAWITEMSTRUCT* p_di) {}
  virtual void OnMenuSelect(int p_menu_id, int p_menu_flag, HMENU p_menu_handle) {}

  virtual void OnClear() {}

// member function
public:
  void ScrollTo(u32 p_line);

protected:
  void AddCol(const char* p_label, int p_width, bool p_fix, QSORT_FUNC (*p_sort_func)(bool));
  int  GetColLeft(char* p_label);
  int  GetColLeft(int p_idx);
  int  GetPointingColBorder(int p_x, int p_y);

  void DrawListFrame();
  void DrawAddress(HDC p_hdc, RECT* p_rect, u32 p_va);

  void CursorUp(int p_val);
  void CursorDown(int p_val);

// static function
public:
  static LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

// accessor
public:
  inline ds_dialog* dlg() { return m_dlg; }
  inline int        select() { return m_sel; }

// member variable
protected:
  ds_dialog*  m_dlg;
  int      m_drag_separater;

  int      m_head;
  int      m_sel;
  int      m_line_count;
  vector<ListWindowCol*> m_col_array;
};

#endif // _debugger_gui_listwindowbasex
