/*---------*/
/* include */
/*---------*/
#include <wx/rawbmp.h>
#include <wx/event.h>

#include "ggscfg.h"
#include "ggscfg_ColorEditSheet.h"
#include "ggscfg_RGBEditDialog.h"

#include "device/graphic.h"

#include "ds_zlib.h"
#include "ds_sharedmemory.h"

int kCharaCount = 0;
const CharaInfo* kCharaInfo = NULL;
int kPaletteCount= 0;
const wxString* kPaletteLabel = NULL;

/*----------*/
/* function */
/*----------*/
ggscfg_ColorEditSheet::ggscfg_ColorEditSheet(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: ColorEditSheet(parent, id, pos, size, style)
{
  m_image = NULL;
  m_image_w = 0;
  m_image_h = 0;

  m_palette_size = 0;
  memset(m_palette, 0, sizeof(u32) * 256);
  memset(m_palette_header, 0, sizeof(u32) * 16);

  memset(m_image_addr_list, 0, sizeof(u32) * kImageMaxCount);
  m_image_addr_list_count = 0;

  memset(m_pal_addr_list, 0, sizeof(u32) * kPaletteMaxCount);
  m_pal_addr_list_count = 0;

  m_cur_cid    = 0;
  m_cur_palette  = 0;
  m_palette_changed = false;
  m_select_palette = -1;

  // Connect Events
  this->Connect(wxID_ANY, wxEVT_TIMER, wxTimerEventHandler(ggscfg_ColorEditSheet::on_timer));
  this->Connect(wxID_ANY, wxEVT_DROP_FILES, wxDropFilesEventHandler(ggscfg_ColorEditSheet::on_drop_file));
}

ggscfg_ColorEditSheet::~ggscfg_ColorEditSheet() {
  if (m_image) delete m_image;
}

void ggscfg_ColorEditSheet::on_init(wxInitDialogEvent& event) {
  if (strcmp(GET_APP->get_title_label(), "SLPS_251.84") == 0) {
    kCharaCount = kCharaCount_251_84;
    kCharaInfo = kCharaInfo_251_84;
    kPaletteCount= kPaletteCount_251_84;
    kPaletteLabel = kPaletteLabel_251_84;
  } else if (strcmp(GET_APP->get_title_label(), "SLPS_252.61") == 0) {
    kCharaCount = kCharaCount_252_61;
    kCharaInfo = kCharaInfo_252_61;
    kPaletteCount= kPaletteCount_252_61;
    kPaletteLabel = kPaletteLabel_252_61;
  } else if (strcmp(GET_APP->get_title_label(), "SLPM_663.33") == 0) {
    kCharaCount = kCharaCount_663_33;
    kCharaInfo = kCharaInfo_663_33;
    kPaletteCount= kPaletteCount_663_33;
    kPaletteLabel = kPaletteLabel_663_33;
  } else if (strcmp(GET_APP->get_title_label(), "SLPM_669.65") == 0) {
    kCharaCount = kCharaCount_669_65;
    kCharaInfo = kCharaInfo_669_65;
    kPaletteCount= kPaletteCount_669_65;
    kPaletteLabel = kPaletteLabel_669_65;
  }
  // キャラコンボの初期化
  for (int i = 0; i < kCharaCount; i++) {
    m_choice_chara->Append(kCharaInfo[i].name);
  }
  m_choice_chara->Select(0);
  
  // カラーコンボの初期化
  for (int i = 0; i < kPaletteCount; i++) {
    m_choice_color->Append(kPaletteLabel[i]);
  }
  m_choice_color->Select(0);

  change_color_edit_chara(0);  // sol
  read_image_data(0, 0);

  // 20msに１回描画
  m_timer.SetOwner(this, wxID_ANY);
  m_timer.Start(20, wxTIMER_CONTINUOUS);

  DragAcceptFiles(true);
}

void ggscfg_ColorEditSheet::on_choice_chara(wxCommandEvent& event) {
  int cid = m_choice_chara->GetSelection();
  int palidx = m_choice_color->GetSelection();

  if (cid != m_cur_cid) {
    int ret = check_save_palette();
    if (ret == 1) {
      // yes
      write_palette();
      m_palette_changed = false;
    } else if (ret == 0) {
      // cancel
      m_choice_chara->SetSelection(m_cur_cid);
      return;
    }
    change_color_edit_chara(cid);
    read_image_data(0, palidx);
    update_ggxx_palette(false);
  }
}

void ggscfg_ColorEditSheet::on_choice_color(wxCommandEvent& event) {
  int imgidx = m_choice_id->GetSelection();
  int palidx = m_choice_color->GetSelection();
  
  if (palidx != m_cur_palette) {
    int ret = check_save_palette();
    if (ret == 1) {
      // yes
      write_palette();
      m_palette_changed = false;
    } else if (ret == 0) {
      // cancel
      m_choice_color->SetSelection(m_cur_palette);
      return;
    }
    read_image_data(imgidx, palidx);
    update_ggxx_palette(false);
  }
}

void ggscfg_ColorEditSheet::on_choice_id(wxCommandEvent& event) {
  int imgidx = m_choice_id->GetSelection();
  read_image_data(imgidx, -1);
}

void ggscfg_ColorEditSheet::on_button_clicked_save_palette(wxCommandEvent& event) {
  write_palette();
}

void ggscfg_ColorEditSheet::on_button_clicked_revert_palette(wxCommandEvent& event) {

}

void ggscfg_ColorEditSheet::on_left_down(wxMouseEvent& event) {
  if (m_select_palette != -1) {
    wxPoint pt = wxGetMousePosition();

    ggscfg_RGBEditDialog rgbdlg(this);
    rgbdlg.InitDialog();
    rgbdlg.SetPosition(wxPoint(pt.x - 150, pt.y - 150));
    rgbdlg.ShowModal();
  }
}

void ggscfg_ColorEditSheet::on_paint_palette(wxPaintEvent& event) {
  draw_palette(&wxPaintDC((wxWindow*)event.GetEventObject()));
}

void ggscfg_ColorEditSheet::on_paint_sprite(wxPaintEvent& event) {
  draw_image(&wxPaintDC((wxWindow*)event.GetEventObject()), false);
}

void ggscfg_ColorEditSheet::on_timer(wxTimerEvent& event) {

  if (this->IsShown() == false) {
    return;
  }

    static int pulse = 0;
  pulse = pulse++ % 10;
  static bool oldinv = 0;

  wxPoint pt = wxGetMousePosition();

  // 選択パレット変更
  wxPoint pal_pt = ScreenToClient(pt);
  wxRect bounds = m_panel_palette->GetRect();
  bounds.Deflate(1, 1);
  if (bounds.Contains(pal_pt)) {
    m_select_palette = ((pal_pt.x - m_panel_palette->GetRect().GetLeft()) / 8) + ((pal_pt.y - m_panel_palette->GetRect().GetTop()) & ~7);
    if (m_select_palette < 0 || m_select_palette >= m_palette_size) {
      m_select_palette = -1;
    }
  } else {
    m_select_palette = -1;
  }

  // 画像からも選択パレット変更する
  wxPoint img_pt = m_scrolledWindow_sprite->ScreenToClient(pt);
  if (img_pt.x >= 0 && img_pt.x < m_image_w && img_pt.y >= 0 && img_pt.y < m_image_h) {
    if (m_palette_size == 16) {
      if (img_pt.x & 1) {
        m_select_palette = (m_image[img_pt.x / 2 + img_pt.y * m_image_w] >> 4) & 0x0f;
      } else {
        m_select_palette =  m_image[img_pt.x / 2 + img_pt.y * m_image_w] & 0x0f;
      }
    } else {
      m_select_palette = m_image[img_pt.x + img_pt.y * m_image_w];
    }
  }

  bool inv = (pulse >= 8) && (wxGetActiveWindow() == GetParent()->GetParent()) && (m_select_palette != -1);
  draw_image(&wxClientDC(m_scrolledWindow_sprite), inv);
  draw_palette(&wxClientDC(m_panel_palette));

  if (oldinv != inv) {
    update_ggxx_palette(inv ? m_select_palette : -1);
  }
  oldinv = inv;
}

void ggscfg_ColorEditSheet::on_drop_file(wxDropFilesEvent &event) {
  if (event.m_noFiles > 0
  &&  read_palette(event.m_files[0])) {
    set_palette_changed(true);
  }
}

bool ggscfg_ColorEditSheet::on_dialog_ok(wxCommandEvent& event) {
  int ret = check_save_palette();
  if (ret == 1) {
    // yes
    write_palette();
  } else if (ret == 0) {
    // cancel
    m_choice_color->SetSelection(m_cur_palette);
    return false;
  }
  return true;
}

bool ggscfg_ColorEditSheet::on_dialog_cancel(wxCommandEvent& event) {
  return true;
}

void ggscfg_ColorEditSheet::change_color_edit_chara(int p_cid) {
  m_palette_changed = false;
  m_cur_cid = p_cid;

  clear_image_offset();

  add_image_offset(kCharaInfo[p_cid].file_ofs);

  // Freeze-Thawで描画を抑止することで高速化する
  m_choice_id->Freeze();
  m_choice_id->Clear();
  for (int i = 0; i < m_image_addr_list_count; i++) {
    char c[10];
    _itoa(i, c, 10);
    m_choice_id->Append(c);
  }
  m_choice_id->Select(0);
  m_choice_id->Thaw();
}

void ggscfg_ColorEditSheet::clear_image_offset(void) {
  m_image_addr_list_count = 0;
  m_pal_addr_list_count = 0;
}

void ggscfg_ColorEditSheet::add_image_offset(int p_offset) {

  s8  cddata_path[1024];
  FILE *fp = fopen(get_cddata_path(cddata_path), "rb");
  if (fp) {
    DWORD  data1, data2;
    int    actualsize;

    fseek(fp, p_offset, SEEK_SET);
    fread(&data1, 1, 4, fp);
    // image data
    fseek(fp, p_offset + data1 + 4, SEEK_SET);
    fread(&data2, 1, 4, fp);
    fseek(fp, p_offset + data1 + data2, SEEK_SET);
    actualsize = fread(&m_image_addr_list[m_image_addr_list_count], 1, 4 * (kImageMaxCount - m_image_addr_list_count), fp);
    
    for (int i = 0; i < actualsize / 4; i++) {
      if (m_image_addr_list[m_image_addr_list_count] == 0xffffffff) break;
      m_image_addr_list[m_image_addr_list_count] += p_offset + data1 + data2;
      m_image_addr_list_count++;
    }
    // pal data
    fseek(fp, p_offset + data1 + 12, SEEK_SET);
    fread(&data2, 1, 4, fp);
    fseek(fp, p_offset + data1 + data2, SEEK_SET);
    actualsize = fread(m_pal_addr_list, 1, sizeof(u32) * kPaletteCount, fp);
    assert(actualsize == sizeof(u32) * kPaletteCount);

    m_pal_addr_list_count = 0;
    for (int i = 0; i < kPaletteCount; i++) {
      if (m_pal_addr_list[m_pal_addr_list_count] == 0xffffffff) break;
      m_pal_addr_list[m_pal_addr_list_count] += p_offset + data1 + data2;
      m_pal_addr_list_count++;
    }
    fclose(fp);
  }
}

bool ggscfg_ColorEditSheet::is_file_exist(const char* p_path) {
  FILE* fp = fopen(p_path, "rb");
  if (fp) {
    fclose(fp);
    return true;
  }
  return false;
}

s8* ggscfg_ColorEditSheet::get_palette_dir(char* p_buf) {
  sprintf(p_buf, "data\\%s\\%s", GET_APP->get_title_label(), kPaletteDir);
  return p_buf;
}

s8* ggscfg_ColorEditSheet::get_palette_path(char* p_buf, int p_cid, int p_pal_idx) {
  get_palette_dir(p_buf);
  sprintf(p_buf, "%s\\%s_%s.pal", p_buf, kCharaInfo[p_cid].name, kPaletteLabel[p_pal_idx]);
  return p_buf;
}

s8* ggscfg_ColorEditSheet::get_cddata_path(char* p_buf) {
  s8 data_file_name[256];
  if (strcmp(GET_APP->get_title_label(), "SLPS_251.84") == 0) {
    strcpy(data_file_name, "cddata.bin");
  } else if (strcmp(GET_APP->get_title_label(), "SLPS_252.61") == 0) {
    strcpy(data_file_name, "bcddata.bin");
  } else if (strcmp(GET_APP->get_title_label(), "SLPM_663.33") == 0) {
    strcpy(data_file_name, "scddata.bin");
  } else if (strcmp(GET_APP->get_title_label(), "SLPM_669.65") == 0) {
    strcpy(data_file_name, "acddata.bin");
  }
  sprintf(p_buf, "data\\%s\\%s", GET_APP->get_title_label(), data_file_name);
  return p_buf;
}

bool ggscfg_ColorEditSheet::read_palette(wxString p_pal_file) {
  bool result = false;

  FILE *fp = fopen(p_pal_file, "rb");
  if (fp) {
    char buf[16 + 256 * 4];
    int size = ds_zlib::zfsize(fp);
    if (size == 0x410) {
      ds_zlib::zfread(buf, 16 + 256 * 4, fp);
      memcpy(m_palette_header, &buf[0], 16);
      memcpy(m_palette, &buf[16], 256 * 4);
      m_palette_size = 256;
      result = true;
    }
    fclose(fp);
  }
  return result;
}

void ggscfg_ColorEditSheet::write_palette(void) {

  s8 str[1024];
  CreateDirectory(get_palette_dir(str), NULL);

  FILE *fp = fopen(get_palette_path(str, m_cur_cid, m_cur_palette), "wb");
  if (fp) {
    char buf[16 + 256 * 4];
    memcpy(&buf[0], m_palette_header, 16);
    memcpy(&buf[16], m_palette, 256 * 4);
    ds_zlib::zfwrite(buf, 16 + 256 * 4, fp);
    fclose(fp);
  }
}

void ggscfg_ColorEditSheet::read_image_data(int p_imgidx, int p_palidx) {
  if (p_palidx != -1) {
    m_palette_changed = false;
    m_cur_palette = p_palidx;
  }
  if (p_imgidx < 0 || p_imgidx >= m_image_addr_list_count) {
    return;
  }
  if (p_palidx < -1 || p_palidx >= kPaletteCount) {
    return;
  }

  s8  cddata_path[1024];
  FILE *fp = fopen(get_cddata_path(cddata_path), "rb");
  if (fp) {
    char header[16];

    fseek(fp, m_image_addr_list[p_imgidx], SEEK_SET);

    fread(header, 1, 16, fp);
    m_palette_size = 1 << header[4];
    m_image_w = *((WORD*)&header[6]);
    m_image_h = *((WORD*)&header[8]);
    
    if (p_palidx != -1) {
      char str[1024];
      if (is_file_exist(get_palette_path(str, m_cur_cid, m_cur_palette))) {
        // fileからpalette読み込み
        read_palette(get_palette_path(str, m_cur_cid, m_cur_palette));
      } else {
        // cddataからデフォルトを読み込み
        if (header[2] == 0x20) {
          memset(m_palette_header, 0, 16);
          memset(m_palette, 0, 256 * 4);
          fread(m_palette, 4, m_palette_size, fp);
        } else {
          fpos_t pos;
          fgetpos(fp, &pos);
          
          fseek(fp, m_pal_addr_list[p_palidx], SEEK_SET);
          fread(m_palette_header, 1, 16, fp);

          fseek(fp, m_pal_addr_list[p_palidx] + 16, SEEK_SET);
          fread(m_palette, 1, 256 * 4, fp);
          
          // convert
          DWORD temp[8];
          for (int i = 0; i < 8; i++) {
            memcpy(temp, &m_palette[8+32*i], 32);
            memcpy(&m_palette[8+32*i], &m_palette[16+32*i], 32);
            memcpy(&m_palette[16+32*i], temp, 32);
          }
          fseek(fp, (int)pos, SEEK_SET);
        }
      }
    } else if (header[2] == 0x20) {
      // パレットなどがあれば飛ばす
      fseek(fp, m_palette_size * 4, SEEK_CUR);
    }

    int imgsize = m_image_w * m_image_h * 2;
    BYTE* srcbuf = new BYTE[imgsize];  // srcのサイズを求めるのが面倒なので、てきとーに十分なサイズを確保
    fread(srcbuf, 1, imgsize, fp);
    fclose(fp);

    if (m_image) {
      delete m_image;
    }
    m_image = new u8[imgsize];
    memset(m_image, 0, imgsize);

    BYTE* dstbuf = m_image;
    if (header[0] == 1) {
      GraphicMgr::decode1(dstbuf, srcbuf);
    } else if (header[0] == 5) {
      GraphicMgr::decode5(dstbuf, srcbuf);
    } else {
      wxLogMessage("未知の形式 = %d\n", header[0]);
    }
    delete srcbuf;
  }
  m_palette_changed = false;
}

int ggscfg_ColorEditSheet::check_save_palette(void) {
  if (m_palette_changed) {
    wxMessageDialog msgdlg(NULL, "Do you save changed palette?", "", wxYES_DEFAULT | wxYES_NO | wxCANCEL | wxICON_INFORMATION);
    switch (msgdlg.ShowModal()) {
    case wxID_YES:    return 1; // save
    case wxID_NO:    return 2; // no save
    case wxID_CANCEL:  return 0; // cancel
    }
  }
  return 2;
}

u32 convert_rgba(u32 p_src) {
  u8* s = (u8*)&p_src;
  return (s[0] << 16) | (s[1] << 8) | s[2] | 0x80000000;
}

void ggscfg_ColorEditSheet::draw_image(wxDC* p_dc, bool p_inv) {

  wxSize size = GetClientSize();

  // ワーク用DCを作成
  wxMemoryDC work_dc;
  wxBitmap bmp(kMaxImageWidth, kMaxImageHeight, 24);
  work_dc.SelectObject(bmp);

  wxNativePixelData pixel_data(bmp);
  assert(pixel_data);

  wxNativePixelData::Iterator p = pixel_data.GetPixels();
  
  #define STORE_PIXEL(_color) {      \
    q.Red() = (_color >> 16) & 0xff;  \
    q.Green() = (_color >> 8) & 0xff;  \
    q.Blue() = _color & 0xff;      \
    q.OffsetX(pixel_data, 1);      \
  }

  int count = 0;
  u32 color;
  if (m_palette_size == 16) {
    
    for (int i = 0; i < m_image_h; i++) {
      if (i >= kMaxImageHeight) continue;
      
      wxNativePixelData::Iterator q = p;

      for (int j = 0; j < m_image_w; j += 2) {
        if (j >= kMaxImageWidth) continue;

        u8 pixel = m_image[count++];

        color = convert_rgba(m_palette[pixel & 0x0f]);
        if (p_inv && m_select_palette == pixel) color = ~color;
        STORE_PIXEL(color);

        color = convert_rgba(m_palette[(pixel >> 4) & 0x0f]);
        if (p_inv && m_select_palette == pixel) color = ~color;
        STORE_PIXEL(color);
      }
      p.OffsetY(pixel_data, 1);
    }
  } else {
    int count = 0;
    for (int i = 0; i < m_image_h; i++) {
      if (i >= kMaxImageHeight) continue;
      
      wxNativePixelData::Iterator q = p;

      for (int j = 0; j < m_image_w; j++) {
        if (j >= kMaxImageWidth) continue;

        u8 pixel = m_image[count++];

        color = convert_rgba(m_palette[pixel]);
        if (p_inv && m_select_palette == pixel) color = ~color;
        STORE_PIXEL(color);
      }
      p.OffsetY(pixel_data, 1);
    }
  }
  // ウインドウに転送
  p_dc->Blit(0, 0, size.x, size.y, &work_dc, 0, 0);
}

void ggscfg_ColorEditSheet::draw_palette(wxDC* p_dc) {

  wxSize size = GetClientSize();

  // ワーク用DCを作成
  wxMemoryDC work_dc;
  wxBitmap bmp(size.x, size.y);
  work_dc.SelectObject(bmp);

  // color cell
  work_dc.SetPen(*wxTRANSPARENT_PEN);
  for (int i = 0; i < m_palette_size; i++) {
    work_dc.SetBrush(wxBrush(wxColour(m_palette[i]), wxSOLID));
    work_dc.DrawRectangle(
      (i % 8) * 8,
      (i / 8) * 8,
      9,
      9);
  }

  // frame
  work_dc.SetPen(*wxBLACK_PEN);
  for (int i = 0; i <= 8; i++) {
    for (int j = 0; j <= 32; j++) {
      work_dc.DrawLine(0, j * 8, 64, j * 8);
    }
    work_dc.DrawLine(i * 8, 0, i * 8, 256);
  }

  // highlight
  if (m_select_palette != -1) {
    work_dc.SetPen(wxPen(*wxRED, 2, wxSOLID));
    work_dc.SetBrush(*wxTRANSPARENT_BRUSH);
    
    work_dc.DrawRectangle(
      (m_select_palette % 8) * 8 + 1,
      (m_select_palette / 8) * 8 + 1,
      8,
      8);
  }

  // ウインドウに転送
  p_dc->Blit(0, 0, size.x, size.y, &work_dc, 0, 0);
}

void ggscfg_ColorEditSheet::update_ggxx_palette(int p_flash) {
  ds_sharedmemory  sm_pal("ggspc_pal", 2048);
  char val;
  /* キャラID */
  val = m_cur_cid;
  sm_pal.set(&m_cur_cid, 1, 1);
  /* キャラID */
  val = m_cur_palette;
  sm_pal.set(&val, 2, 1);
  /* パレットヘッダー、データ */
  sm_pal.set(m_palette_header, 3, 16);
  sm_pal.set(m_palette, 16 + 3, 1024);
  /* 点滅カラー */
  if (p_flash != -1) {
    DWORD fpal = ~m_palette[p_flash];
    sm_pal.set(&fpal, 16 + 3 + 4 * p_flash, 4);
  }
  /* 更新フラグ立て */
  val = 1;
  sm_pal.set(&val, 0, 1);
}
