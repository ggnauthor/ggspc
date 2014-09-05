#ifndef __ggscfg_GeneralSheet__
#define __ggscfg_GeneralSheet__

/*---------*/
/* include */
/*---------*/
#include <wx/wxprec.h>

#include "main.h"
#include "ggscfg_gui.h"
#include "ggscfg_ConfigDialog.h"

/*--------*/
/* define */
/*--------*/

/*-------*/
/* class */
/*-------*/
/** Implementing GeneralSheet */
class ggscfg_GeneralSheet : public GeneralSheet
{
  friend BOOL CALLBACK enum_monitor_callback(HMONITOR p_hmon, HDC p_hdc, LPRECT p_rect, LPARAM p_data);

protected:
  // Handlers for GeneralSheet events.
  void on_init(wxInitDialogEvent& event);
  void on_choice_device(wxCommandEvent& event);
  void on_left_down(wxMouseEvent& event);
  void on_scroll_volume(wxScrollEvent& event);
  void on_text_volume(wxCommandEvent& event);
  void on_timer(wxTimerEvent& event);
  void onchoice_screen_mode(wxCommandEvent& event);

public:
  enum {
    kKeyCfgCtrlDefaultColor = 0xffffff,
    kKeyCfgCtrlDisableColor = 0xc0c0c0,
    kKeyCfgCtrlHighlightColor = 0xa0a0ff,
  };

  /** Constructor */
  ggscfg_GeneralSheet(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1, -1), long style = wxTAB_TRAVERSAL);
  ~ggscfg_GeneralSheet();

  bool on_dialog_ok(wxCommandEvent& event);
  bool on_dialog_cancel(wxCommandEvent& event);

  void update_keyctrls(void);
  void set_screen_mode_enable();
  inline wxChoice* get_choice_device_ctrl(int p_side) { return p_side == 0 ? m_choice_device1P : m_choice_device2P; }

private:
  wxTimer    m_timer;
  wxTextCtrl*  m_keycfg_target;
  int      m_keycfg_target_side;
  int      m_keycfg_target_idx;
  
  wxTextCtrl*  m_keycfg_ctrl_list[2][16];

  class InputMgr*  m_input_mgr;
};

#endif // __ggscfg_GeneralSheet__
