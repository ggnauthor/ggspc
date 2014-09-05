#ifndef _debugger_gui_referencewindow
#define _debugger_gui_referencewindow

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
class ReferenceWindow : public ListWindowBase {
// constant definition
public:
  enum {
    kCtrlPnlHeight  = 22,
    kLabelHeight  = 16,

#ifdef _ENV2
    kViewX = 710 + 150,
    kViewY = 280 + 40,
    kViewW = 460,
    kViewH = 280,
#else
    kViewX = 868,
    kViewY = 334,
    kViewW = 680,
    kViewH = 348,
#endif

    kTreeBtnLeft  = 3,
    kTreeBtnTop    = 2,
    kTreeBtnWidth  = 9,
    kTreeBtnHeight  = 9,
  };

  enum {
    kCat_Comment = 0,
    kCat_Label,
    kCat_MemLabel,
    kCat_String,
    kCat_FindResult,
    kCat_BottleNeck,
  };
  
  enum {
    kRowType_None = 0,
    kRowType_Root,
    kRowType_Array,
    kRowType_Element,
  };

  enum {
    kPopupMenu_SearchReference = WM_USER,
    kPopupMenu_ShowPointer,
  };

// constructor/destructor
public:
  ReferenceWindow();
  ~ReferenceWindow();

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
  virtual void SortListData(QSORT_FUNC p_sort_func);

private:
  void Draw_Comment(HDC p_hdc);
  void Draw_Label(HDC p_hdc);
  void Draw_MemLabel(HDC p_hdc);
  void Draw_String(HDC p_hdc);
  void Draw_FindResult(HDC p_hdc);
  void Draw_BottleNeck(HDC p_hdc);

// event handler
public:
  //void OnInitDialog(HWND p_hwnd);
  //void OnActivate(int p_active, int p_minimized, int p_hwnd_pre);
  void OnCommand(int p_ctrlID, int p_notify);
  void OnCategoryComboSelChange();
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
  //void OnDrawItem(int p_ctrlID, DRAWITEMSTRUCT* p_di);
  //void OnMenuSelect(int p_menu_id, int p_menu_flag, HMENU p_menu_handle);

  void OnClear();

// member function
public:
  void add_entry(Comment* p_data);
  void add_entry(Label* p_data);
  void add_entry(MemLabel* p_data);
  void add_entry(BottleNeck* p_data);
  void remove_entry(Comment* p_data);
  void remove_entry(Label* p_data);
  void remove_entry(MemLabel* p_data);
  void update_list();
  void update_filter(const char* p_filter_str = NULL);

  void SearchReference(u32 p_va);
  void UpdateBottleNeckEntries();
  //void UpdateDisplayList(const char* p_str = NULL);

private:
  int  GetExpandedRowCount();
  int  GetExpandedRowIndex(int p_row, int* p_idx, int* p_ary_idx, int* p_elem_idx);
  void SelectExpandedRow(int p_idx, int p_ary_idx);

  void DrawTreeBtn(HDC p_hdc, int p_left, int p_top, bool p_expand);
  bool IsPointTreeBtn(int p_left, int p_top, int p_x, int p_y);
  void ShowPointer();

// member variable
private:
  int  m_category;
  bool m_show_cur_only;
  vector<ReferenceEntryBase*>  m_sorted_ary;
  vector<ReferenceEntryBase*>  m_filtered_ary;
};

#endif // _debugger_gui_referencewindow
