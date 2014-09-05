#ifndef __ggscfg_NetworkSheet__
#define __ggscfg_NetworkSheet__

/*---------*/
/* include */
/*---------*/
#include "ggscfg_gui.h"
#include "ggscfg_ConfigDialog.h"

const char kRankChars[7] = {
  'F', 'E', 'D', 'C', 'B', 'A', 'S',
};

/*-------*/
/* class */
/*-------*/
/** Implementing NetworkSheet */
class ggscfg_NetworkSheet : public NetworkSheet
{
protected:
  // Handlers for NetworkSheet events.
  void on_init(wxInitDialogEvent& event);
  void on_button_clicked_lobby_address_add( wxCommandEvent& event );
  void on_button_clicked_lobby_address_del( wxCommandEvent& event );
  void on_button_clicked_edit_comment( wxCommandEvent& event );
  
public:
  /** Constructor */
  ggscfg_NetworkSheet(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1, -1), long style = wxTAB_TRAVERSAL);
  ~ggscfg_NetworkSheet();

  bool on_dialog_ok(wxCommandEvent& event);
  bool on_dialog_cancel(wxCommandEvent& event);
};

#endif // __ggscfg_NetworkSheet__
