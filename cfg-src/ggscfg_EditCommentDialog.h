#ifndef __ggscfg_EditCommentDialog__
#define __ggscfg_EditCommentDialog__

/*---------*/
/* include */
/*---------*/
#include "ggscfg_gui.h"
#include "ggscfg_ConfigDialog.h"

/*-------*/
/* class */
/*-------*/
/** Implementing EditCommentDialog */
class ggscfg_EditCommentDialog : public EditCommentDialog
{
protected:
  // Handlers for EditCommentDialog events.
  void on_ok(wxCommandEvent& event);
  void on_cancel(wxCommandEvent& event);
  
public:
  /** Constructor */
  ggscfg_EditCommentDialog(wxWindow* parent);
};

#endif // __ggscfg_EditCommentDialog__
