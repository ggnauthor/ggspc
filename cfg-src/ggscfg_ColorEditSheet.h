#ifndef __ggscfg_ColorEditSheet__
#define __ggscfg_ColorEditSheet__

/*---------*/
/* include */
/*---------*/
#include <wx/wxprec.h>

#include "main.h"
#include "ggscfg_gui.h"

/*--------*/
/* define */
/*--------*/
struct CharaInfo {
  wxUint32  file_ofs;
  wxString  name;
};

#define kCharaCount_251_84  23
const CharaInfo kCharaInfo_251_84[] = {
  0x085bf800, "Sol",
  0x0898d000, "Ky",
  0x08d4a000, "May",
  0x0913d000, "Millia",
  0x093e5000, "Axl",
  0x09742000, "Potemkin",
  0x09be3000, "Chipp",
  0x09eab800, "Eddie",
  0x0a19b000, "Baiken",
  0x0a4ab000, "Faust",
  0x0a8d2000, "Testament",
  0x0acf7000, "Jam",
  0x0b029800, "Anji",
  0x0b415000, "Johnny",
  0x0b7da000, "Venom",
  0x0ba52800, "Dizzy",
  0x0bf88000, "Slayer",
  0x0c48a800, "I-No",
  0x0c99d000, "Zappa",
  0x0ce95800, "Bridget",
  0x0d269000, "Robo-Ky",
  0x0d612000, "Kliff",
  0x0d8b1800, "Justice",
};

#define kCharaCount_252_61  23
const CharaInfo kCharaInfo_252_61[] = {
  0x08824800, "Sol",
  0x08bf2000, "Ky",
  0x08faf800, "May",
  0x093a4000, "Millia",
  0x0964c000, "Axl",
  0x099a9000, "Potemkin",
  0x09e4a000, "Chipp",
  0x0a112800, "Eddie",
  0x0a402000, "Baiken",
  0x0a712000, "Faust",
  0x0ab39000, "Testament",
  0x0af5e000, "Jam",
  0x0b2c8000, "Anji",
  0x0b6b3800, "Johnny",
  0x0ba79000, "Venom",
  0x0bcf2800, "Dizzy",
  0x0c228000, "Slayer",
  0x0c727800, "I-No",
  0x0cc3a000, "Zappa",
  0x0d132800, "Bridget",
  0x0d505800, "Robo-Ky",
  0x0d941800, "Kliff",
  0x0dbe1000, "Justice",
};

#define kCharaCount_663_33  25
const CharaInfo kCharaInfo_663_33[] = {
  0x9a79800, "Sol",
  0x9e5d000, "Ky",
  0xa221000, "May",
  0xa62b000, "Millia",
  0xa8d6800, "Axl",
  0xac56800, "Potemkin",
  0xb117000, "Chipp",
  0xb3fc000, "Eddie",
  0xb6fa800, "Baiken",
  0xba0f800, "Faust",
  0xbe3f000, "Testament",
  0xc27c800, "Jam",
  0xc5e9800, "Anji",
  0xc9ec000, "Johnny",
  0xcd84800, "Venom",
  0xd002800, "Dizzy",
  0xd53d000, "Slayer",
  0xda40000, "I-No",
  0xdf54800, "Zappa",
  0xe456800, "Bridget",
  0xe83a800, "Robo-Ky",
  0xec8d800, "A.B.A",
  0xf19b000, "Order-Sol",
  0xf6e0000, "Kliff",
  0xf985000, "Justice",
};

#define kCharaCount_669_65  25
const CharaInfo kCharaInfo_669_65[] = {
  0x103bc800, "Sol",
  0x107a0800, "Ky",
  0x10bab800, "May",
  0x10fcd800, "Millia",
  0x11274800, "Axl",
  0x11600000, "Potemkin",
  0x11abe000, "Chipp",
  0x11d85800, "Eddie",
  0x12079800, "Baiken",
  0x1239c800, "Faust",
  0x127c2800, "Testament",
  0x12c02000, "Jam",
  0x12f6e800, "Anji",
  0x1339c800, "Johnny",
  0x13742800, "Venom",
  0x139c7000, "Dizzy",
  0x13ef9800, "Slayer",
  0x14434800, "I-No",
  0x14928800, "Zappa",
  0x14e43000, "Bridget",
  0x151e1800, "Robo-Ky",
  0x15634000, "A.B.A",
  0x15b57800, "Order-Sol",
  0x16089800, "Kliff",
  0x16328800, "Justice",
};

#define  kPaletteCount_251_84  12
const wxString kPaletteLabel_251_84[kPaletteCount_251_84] = {
  "P",
  "K",
  "S",
  "HS",
  "Start",
  "D",
  "P(Sp)",
  "K(Sp)",
  "S(Sp)",
  "HS(Sp)",
  "Start(Sp)",
  "D(Sp)",
};

#define  kPaletteCount_252_61  12
const wxString kPaletteLabel_252_61[kPaletteCount_252_61] = {
  "P",
  "K",
  "S",
  "HS",
  "Start",
  "D",
  "P(Sp)",
  "K(Sp)",
  "S(Sp)",
  "HS(Sp)",
  "Start(Sp)",
  "D(Sp)",
};

#define  kPaletteCount_663_33  24
const wxString kPaletteLabel_663_33[kPaletteCount_663_33] = {
  "P(#R)",
  "K(#R)",
  "S(#R)",
  "HS(#R)",
  "Start(#R)",
  "D(#R)",
  "P(#R Sp)",
  "K(#R Sp)",
  "S(#R Sp)",
  "HS(#R Sp)",
  "Start(#R Sp)",
  "D(#R Sp)",
  "P",
  "K",
  "S",
  "HS",
  "Start",
  "D",
  "P(Sp)",
  "K(Sp)",
  "S(Sp)",
  "HS(Sp)",
  "Start(Sp)",
  "D(Sp)",
};

#define  kPaletteCount_669_65  20
const wxString kPaletteLabel_669_65[kPaletteCount_669_65] = {
  "P",
  "K",
  "S",
  "HS",
  "D",
  "P(Ex)",
  "K(Ex)",
  "S(Ex)",
  "HS(Ex)",
  "D(Ex)",
  "P(Slash)",
  "K(Slash)",
  "S(Slash)",
  "HS(Slash)",
  "D(Slash)",
  "P(#R)",
  "K(#R)",
  "S(#R)",
  "HS(#R)",
  "D(#R)",
};

#define kImageMaxCount    2048
#define kPaletteMaxCount  32
#define kMaxImageWidth    600
#define kMaxImageHeight    600

/*-------*/
/* class */
/*-------*/
/** Implementing ColorEditSheet */
class ggscfg_ColorEditSheet : public ColorEditSheet
{
protected:
  // Handlers for ColorEditSheet events.
  void on_init(wxInitDialogEvent& event);
  void on_choice_chara(wxCommandEvent& event);
  void on_choice_color(wxCommandEvent& event);
  void on_choice_id(wxCommandEvent& event);
  void on_button_clicked_save_palette(wxCommandEvent& event);
  void on_button_clicked_revert_palette(wxCommandEvent& event);
  void on_left_down(wxMouseEvent& event);
  void on_paint_palette(wxPaintEvent& event);
  void on_paint_sprite(wxPaintEvent& event);
  void on_timer(wxTimerEvent& event);

  void on_drop_file(wxDropFilesEvent &event);

  void clear_image_offset(void);
  void add_image_offset(int p_offset);
  void change_color_edit_chara(int p_cid);

  bool is_file_exist(const char* p_path);
  s8*  get_palette_dir(char* p_buf);
  s8*  get_palette_path(char* p_buf, int p_cid, int p_pal_idx);
  s8*  get_cddata_path(char* p_buf);

  bool read_palette(wxString p_pal_file);
  void write_palette(void);

  void read_image_data(int p_imgidx, int p_palidx);
  int  check_save_palette(void);
  
  void draw_image(wxDC* p_dc, bool p_inv);
  void draw_palette(wxDC* p_dc);

public:
  /** Constructor */
  ggscfg_ColorEditSheet(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1, -1), long style = wxTAB_TRAVERSAL);
  ~ggscfg_ColorEditSheet();

  bool on_dialog_ok(wxCommandEvent& event);
  bool on_dialog_cancel(wxCommandEvent& event);

  void update_ggxx_palette(int p_flash);

  u32  get_palette_color(int p_idx) { return m_palette[p_idx]; }
  void set_palette_color(int p_idx, u32 p_color) { m_palette[p_idx] = p_color; }

  int get_select_palette(void) { return m_select_palette; }
  void set_palette_changed(bool p_changed) { m_palette_changed = p_changed; }

private:
  wxTimer  m_timer;

  u8*    m_image;
  int    m_image_w;
  int    m_image_h;

  int    m_palette_size;
  u32    m_palette[256];
  u32    m_palette_header[16];

  u32    m_image_addr_list[kImageMaxCount];
  int    m_image_addr_list_count;

  u32    m_pal_addr_list[kPaletteMaxCount];
  int    m_pal_addr_list_count;

  int    m_cur_cid;
  int    m_cur_palette;
  bool  m_palette_changed;
  int    m_select_palette;
};

/*--------*/
/* extern */
/*--------*/
extern int kCharaCount;
extern const CharaInfo* kCharaInfo;
extern int kPaletteCount;
extern const wxString* kPaletteLabel;

#endif // __ggscfg_ColorEditSheet__
