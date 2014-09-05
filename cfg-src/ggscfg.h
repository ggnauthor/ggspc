#ifndef _ggscfg
#define _ggscfg

/*---------*/
/* include */
/*---------*/
#include <wx/wxprec.h>

#include "main.h"

/*-------*/
/* class */
/*-------*/
class GgsCfgApp : public wxApp {
public:
  virtual bool OnInit();
  virtual int  OnExit();

  s8* get_title_label() {
    return m_title_label;
  }

private:
  s8  m_title_label[256];
};

DECLARE_APP(GgsCfgApp)

#define GET_APP ((GgsCfgApp*)&wxGetApp())

#endif // _ggscfg
