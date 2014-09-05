/*---------*/
/* include */
/*---------*/
#include <wx/wxprec.h>

#include "ggscfg_ColorEditSheet.h"
#include "ggscfg_RGBEditDialog.h"

/*----------*/
/* function */
/*----------*/
ggscfg_RGBEditDialog::ggscfg_RGBEditDialog(wxWindow* parent) : RGBEditDialog(parent)
{
  m_color_edit_sheet = dynamic_cast<ggscfg_ColorEditSheet*>(m_parent);
  m_palette_idx = m_color_edit_sheet->get_select_palette();
  m_old_color = m_color_edit_sheet->get_palette_color(m_palette_idx);
  m_new_color = m_old_color;

  update_sliders();
  update_textboxes();
  update_picker();
}

void ggscfg_RGBEditDialog::on_scroll(wxScrollEvent& event) {
  wxSlider* obj = (wxSlider*)event.GetEventObject();
  long tmp = obj->GetValue();
  if (tmp > 255) tmp = 255;
  if (tmp < 0) tmp = 0;

  if (obj == m_sliderR) {
    m_new_color = (m_new_color & 0xffffff00) | tmp;
  } else if (obj == m_sliderG) {
    m_new_color = (m_new_color & 0xffff00ff) | tmp << 8;
  } else if (obj == m_sliderB) {
    m_new_color = (m_new_color & 0xff00ffff) | tmp << 16;
  }
  update_textboxes();
  update_picker();
  update_color();
}

void ggscfg_RGBEditDialog::on_text(wxCommandEvent& event) {
  wxTextCtrl* obj = (wxTextCtrl*)event.GetEventObject();
  long tmp;
  obj->GetValue().ToLong(&tmp);
  if (tmp > 255) tmp = 255;
  if (tmp < 0) tmp = 0;

  if (obj == m_textCtrlR) {
    m_new_color = (m_new_color & 0xffffff00) | tmp;
  } else if (obj == m_textCtrlG) {
    m_new_color = (m_new_color & 0xffff00ff) | tmp << 8;
  } else if (obj == m_textCtrlB) {
    m_new_color = (m_new_color & 0xff00ffff) | tmp << 16;
  }
  update_sliders();
  update_picker();
  update_color();
}

void ggscfg_RGBEditDialog::on_picked(wxColourPickerEvent& event) {
  m_new_color = event.GetColour().Blue() << 16 |
                event.GetColour().Green()  << 8 |
                event.GetColour().Red();
  update_textboxes();
  update_sliders();
  update_color();
}

void ggscfg_RGBEditDialog::on_ok(wxCommandEvent& event) {
  m_color_edit_sheet->set_palette_changed(true);

  Close();
}

void ggscfg_RGBEditDialog::on_cancel(wxCommandEvent& event) {
  m_color_edit_sheet->set_palette_color(m_palette_idx, m_old_color);
  Close();
}

void ggscfg_RGBEditDialog::update_sliders() {
  m_sliderR->SetValue(m_new_color & 0xff);
  m_sliderG->SetValue((m_new_color >> 8) & 0xff);
  m_sliderB->SetValue((m_new_color >> 16) & 0xff);

  m_sliderR->SetPageSize(10);
  m_sliderG->SetPageSize(10);
  m_sliderB->SetPageSize(10);
}

void ggscfg_RGBEditDialog::update_textboxes() {
  wxString tmp;
  tmp.Printf("%d", m_new_color & 0xff);
  m_textCtrlR->SetValue(tmp);
  tmp.Printf("%d", (m_new_color >> 8) & 0xff);
  m_textCtrlG->SetValue(tmp);
  tmp.Printf("%d", (m_new_color >> 16) & 0xff);
  m_textCtrlB->SetValue(tmp);
}

void ggscfg_RGBEditDialog::update_picker() {
  m_colourPicker->SetColour(wxColour(m_new_color));
}

void ggscfg_RGBEditDialog::update_color() {
  m_color_edit_sheet->set_palette_color(m_palette_idx, m_new_color);
  m_color_edit_sheet->update_ggxx_palette(-1);
}
