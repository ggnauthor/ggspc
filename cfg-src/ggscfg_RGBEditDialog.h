#ifndef __ggscfg_RGBEditDialog__
#define __ggscfg_RGBEditDialog__

/*---------*/
/* include */
/*---------*/
#include <wx/wxprec.h>

#include "main.h"
#include "ggscfg_gui.h"

/*-------*/
/* class */
/*-------*/
/** Implementing RGBEditDialog */
class ggscfg_RGBEditDialog : public RGBEditDialog
{
protected:
  // Handlers for RGBEditDialog events.
  void on_scroll(wxScrollEvent& event);
  void on_text(wxCommandEvent& event);
  void on_picked(wxColourPickerEvent& event);
  void on_ok(wxCommandEvent& event);
  void on_cancel(wxCommandEvent& event);
  
  void update_sliders();
  void update_textboxes();
  void update_picker();

  void update_color();

public:
  /** Constructor */
  ggscfg_RGBEditDialog( wxWindow* parent );

private:
  ggscfg_ColorEditSheet* m_color_edit_sheet;
  int  m_palette_idx;
  u32  m_new_color;
  u32  m_old_color;
};

#endif // __ggscfg_RGBEditDialog__
