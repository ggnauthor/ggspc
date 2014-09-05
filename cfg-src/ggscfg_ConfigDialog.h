#ifndef __ggscfg_ConfigDialog__
#define __ggscfg_ConfigDialog__

/*---------*/
/* include */
/*---------*/
#include <wx/wxprec.h>

#include "main.h"
#include "ggscfg_gui.h"

/*-------*/
/* class */
/*-------*/
/** Implementing ConfigDialog */
class ggscfg_ConfigDialog : public ConfigDialog
{
protected:
  // Handlers for ConfigDialog events.
  void on_ok(wxCommandEvent& event);
  void on_cancel(wxCommandEvent& event);
  
  bool value_check(void);

public:
  /** Constructor */
  ggscfg_ConfigDialog(wxWindow* parent);
  ~ggscfg_ConfigDialog();
};

#endif // __ggscfg_ConfigDialog__
