#include "device/graphic.h"

#if !defined _CONFIG
#pragma comment(lib, "d3d9.lib")

#include "main.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "device/device.h"
#include "device/setting.h"
#if defined _DEBUGGER
#include "debugger/debugger.h"
#include "debugger/gui/window/screen_window.h"
#else  // defined _DEBUGGER
#include "window.h"
#endif // defined _DEBUGGER
#include <utils/ds_util.h>

//#define _EXPORT_IMAGE 画像出力テスト
#if defined _EXPORT_IMAGE
#include <ds_image.h>
hash_map<int, int> g_saved_image;
void exportImage(ImageRefData* p_imgRefData, decoded_image* p_img);
#endif // defined _EXPORT_IMAGE

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

GraphicMgr::GraphicMgr() :
    d3d_(NULL),
    d3d_dev_(NULL),
    rendering_now_(false),
    device_lost_(false),
    full_screen_mode_(false),
    render_entry_count_(0),
    save_window_style_(0),
    d3d_vertex_buf_(NULL),
    d3d_index_buf_(NULL),
    d3d_vtx_count_(0),
    d3d_idx_count_(0),
    movie_tex_(NULL) {
  memset(&d3d_display_mode_, 0, sizeof(d3d_display_mode_));
  memset(&d3dpp_window_, 0, sizeof(d3dpp_window_));
  memset(&d3dpp_full_, 0, sizeof(d3dpp_full_));
  memset(&save_window_rect_, 0, sizeof(save_window_rect_));
  memset(render_entry_, 0, sizeof(render_entry_));
}

GraphicMgr::~GraphicMgr() {
}

bool GraphicMgr::init() {
  HRESULT hr;
  const D3DFORMAT d3d_fmt[6] = {
    D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_A2B10G10R10,
    D3DFMT_X1R5G5B5, D3DFMT_A1R5G5B5, D3DFMT_R5G6B5
  };

  d3d_ = Direct3DCreate9(D3D_SDK_VERSION);
  if (d3d_ == NULL) {
    g_scrn->errMsgBox(kErr_InitD3D);
    return false;
  }
  
  // コンフィグで指定したディスプレイデバイスを選択
  int adapter_id = D3DADAPTER_DEFAULT;
  for (u32 i = 0; i < d3d_->GetAdapterCount(); i++) {
    D3DADAPTER_IDENTIFIER9 adapter_info;
    d3d_->GetAdapterIdentifier(i, 0, &adapter_info);
    if (strcmp(g_app.setting_file_mgr()->get_data()->graph_display_device(),
               adapter_info.DeviceName) == 0) {
      adapter_id = i;
      break;
    }
  }

  hr = d3d_->GetAdapterDisplayMode(adapter_id, &d3d_display_mode_);
  if (FAILED(hr)) {
    g_scrn->errMsgBox(kErr_CallCOMFunc, "GetAdapterDisplayMode", hr);
    return false;
  }

  D3DCAPS9 caps;
  hr = d3d_->GetDeviceCaps(adapter_id, D3DDEVTYPE_HAL, &caps);
  if (FAILED(hr)) {
    g_scrn->errMsgBox(kErr_CallCOMFunc, "GetDeviceCaps", hr);
    return false;
  }

  // 画面の物理サイズを取得
  HDC hdc = GetDC(NULL);
  double w = GetDeviceCaps(hdc, HORZSIZE);
  double h = GetDeviceCaps(hdc, VERTSIZE);
  physical_screen_ratio_ = w / h;
  ReleaseDC(NULL, hdc);

  // window mode
  d3dpp_window_.Windowed                   = TRUE;
  d3dpp_window_.BackBufferCount            = 1;
  d3dpp_window_.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
  d3dpp_window_.hDeviceWindow              = g_scrn->hwnd();
  if (g_app.setting_file_mgr()->get_data()->graph_wait_vsync()) {
    d3dpp_window_.PresentationInterval     = D3DPRESENT_INTERVAL_ONE;
  } else {
    d3dpp_window_.PresentationInterval     = D3DPRESENT_INTERVAL_IMMEDIATE;
  }
  d3dpp_window_.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
  d3dpp_window_.BackBufferWidth            = kScreenWidth;
  d3dpp_window_.BackBufferHeight           = kScreenHeight;
  d3dpp_window_.BackBufferFormat           = D3DFMT_UNKNOWN;
  d3dpp_window_.EnableAutoDepthStencil     = FALSE;
  d3dpp_window_.AutoDepthStencilFormat     = D3DFMT_D24X8;
  d3dpp_window_.Flags                      = 0;
  d3dpp_window_.MultiSampleType            = D3DMULTISAMPLE_NONE;

  for (int i = 0; i < 6; i++) {
    hr = d3d_->CheckDeviceType(adapter_id, D3DDEVTYPE_HAL,
                                d3d_display_mode_.Format, d3d_fmt[i], TRUE);
    if (SUCCEEDED(hr)) {
      d3dpp_window_.BackBufferFormat = d3d_fmt[i];
      break;
    }
  }

  // full screen mode
  d3dpp_full_.Windowed                     = FALSE;
  d3dpp_full_.BackBufferCount              = 1;
  d3dpp_full_.SwapEffect                   = D3DSWAPEFFECT_DISCARD;
  d3dpp_full_.hDeviceWindow                = g_scrn->hwnd();
  if (g_app.setting_file_mgr()->get_data()->graph_wait_vsync()) {
    d3dpp_full_.PresentationInterval       = D3DPRESENT_INTERVAL_ONE;
    d3dpp_full_.FullScreen_RefreshRateInHz = 60;
  } else {
    d3dpp_full_.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;
    d3dpp_full_.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
  }
  d3dpp_full_.BackBufferWidth              = 0;
  d3dpp_full_.BackBufferHeight             = 0;
  d3dpp_full_.BackBufferFormat             = D3DFMT_UNKNOWN;
  d3dpp_full_.EnableAutoDepthStencil       = FALSE;
  d3dpp_full_.AutoDepthStencilFormat       = D3DFMT_D24X8;
  d3dpp_full_.Flags                        = 0;
  d3dpp_full_.MultiSampleType              = D3DMULTISAMPLE_NONE;
  
  for (int i = 0; i < 6; i++) {
    hr = d3d_->CheckDeviceType(adapter_id, D3DDEVTYPE_HAL,
                                d3d_fmt[i], d3d_fmt[i], FALSE);
    if (FAILED(hr)) continue;

    int mode_count = d3d_->GetAdapterModeCount(adapter_id, d3d_fmt[i]);
    for (int j = 0; j < mode_count; j++) {
      D3DDISPLAYMODE dm;
      d3d_->EnumAdapterModes(adapter_id, d3d_fmt[i], j, &dm);
      if (g_app.setting_file_mgr()->get_data()->graph_fullscreen() !=
            SettingFileData::kDisplayMode_FullScreenExcl &&
          g_app.setting_file_mgr()->get_data()->graph_keep_aspect_ratio()) {
        u32 width, height;
        static const double req_ratio = (double)kScreenWidth / kScreenHeight;
        if (physical_screen_ratio_ > req_ratio) {
          height = (u32)(dm.Width / physical_screen_ratio_);
          width  = dm.Width;
        } else {
          height = dm.Height;
          width  = (u32)(dm.Height / physical_screen_ratio_);
        }
        if (width  >= kScreenWidth &&
            height >= kScreenHeight &&
            dm.RefreshRate == 60) {
          d3dpp_full_.BackBufferWidth  = dm.Width;
          d3dpp_full_.BackBufferHeight  = dm.Height;
          d3dpp_full_.BackBufferFormat  = dm.Format;
          i = 6;
          j = mode_count;
        }
      } else {
        if (dm.Width  == kScreenWidth &&
            dm.Height == kScreenHeight &&
            dm.RefreshRate == 60) {
          d3dpp_full_.BackBufferWidth  = dm.Width;
          d3dpp_full_.BackBufferHeight  = dm.Height;
          d3dpp_full_.BackBufferFormat  = dm.Format;
          i = 6;
          j = mode_count;
        }
      }
    }
  }

  // 一旦ウインドウモードで起動して、初期ウインドウの位置を自動設定する
  full_screen_mode_ = false;
  hr = d3d_->CreateDevice(
    adapter_id,
    D3DDEVTYPE_HAL,
    g_scrn->hwnd(),
    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
    &d3dpp_window_,
    &d3d_dev_);
  if (FAILED(hr)) {
    g_scrn->errMsgBox(kErr_CallCOMFunc, "CreateDevice", hr);
    return false;
  }

  hr = d3d_dev_->CreateVertexBuffer(
    kMaxVertexBufferEntry * sizeof(D3DVertex),
    D3DUSAGE_WRITEONLY,
    0,
    D3DPOOL_MANAGED,
    &d3d_vertex_buf_,
    NULL);
  if (FAILED(hr)) return false;

  d3d_vtx_count_ = 0;

  hr = d3d_dev_->CreateIndexBuffer(
    kMaxIndexBufferEntry * sizeof(u16),
    D3DUSAGE_WRITEONLY,
    D3DFMT_INDEX16,
    D3DPOOL_MANAGED,
    &d3d_index_buf_,
    0);
  if (FAILED(hr)) return false;

  d3d_idx_count_ = 0;

  // Movie用のテクスチャを専用に用意する
  movie_tex_ = new d3d_texture();
  movie_tex_->id  = 0;
  movie_tex_->ttl = 0;
  movie_tex_->w   = kScreenWidth;
  movie_tex_->h   = kScreenHeight;
  movie_tex_->tw  = 1024;
  movie_tex_->th  = 512;
  hr = d3d_dev_->CreateTexture(
    movie_tex_->tw, movie_tex_->th,
    1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED,
    &movie_tex_->data, NULL);
  if (FAILED(hr)) {
    g_scrn->errMsgBox(kErr_CallCOMFunc, "CreateTexture", hr);
    return false;
  }
  return true;
}

void GraphicMgr::setup() {
  // 固定機能を使用
  d3d_dev_->SetVertexShader(NULL);
  d3d_dev_->SetFVF(kD3DVertexFVF);

  // ずっとこのまま使う
  d3d_dev_->SetIndices(d3d_index_buf_);
  d3d_dev_->SetStreamSource(0, d3d_vertex_buf_, 0, sizeof(D3DVertex));

  d3d_dev_->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);
  d3d_dev_->Present(NULL, NULL, NULL, NULL);

  // アンビエントはライトとマテリアルで指定するので不要
  d3d_dev_->SetRenderState(D3DRS_AMBIENT, 0x00000000);

  // シェーディングはグーローで固定
  // カリングの設定（デフォルトカリングしない）
  d3d_dev_->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
  d3d_dev_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  
  // αブレンディング演算方式
  d3d_dev_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  d3d_dev_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
  d3d_dev_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  d3d_dev_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

  // テクスチャごとのカラーソースを設定（マルチテクスチャは未対応）
  d3d_dev_->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  d3d_dev_->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
  d3d_dev_->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
  d3d_dev_->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
  d3d_dev_->SetTextureStageState(0, D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);
  d3d_dev_->SetTextureStageState(0, D3DTSS_ALPHAARG2,D3DTA_TEXTURE);

  // インデックス頂点ブレンディングを使用しない
  d3d_dev_->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
  d3d_dev_->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
  
  // アンチエイリアス（デフォルト無効）
  d3d_dev_->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, D3DMULTISAMPLE_NONE);

  // 頂点カラーは使用しない
  d3d_dev_->SetRenderState(D3DRS_COLORVERTEX, FALSE);
  d3d_dev_->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
  d3d_dev_->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
  d3d_dev_->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
  d3d_dev_->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);

  // アルファテストでカラーキーにはZ書き込みしない
  d3d_dev_->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
  d3d_dev_->SetRenderState(D3DRS_ALPHAREF, 0x00);
  d3d_dev_->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NEVER);
  
  // Zバッファを無効
  d3d_dev_->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
  d3d_dev_->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
  d3d_dev_->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

  // ステンシルバッファを無効
  d3d_dev_->SetRenderState(D3DRS_STENCILENABLE, FALSE);

  // テクスチャのマッピング方式は常時ラップとし、補間は無効にする
  d3d_dev_->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  d3d_dev_->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
  d3d_dev_->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
  d3d_dev_->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
  
  // ライト無効
  d3d_dev_->SetRenderState(D3DRS_LIGHTING, FALSE);
  d3d_dev_->LightEnable(0, FALSE);
  
  // スペキュラを無効
  d3d_dev_->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
}

void GraphicMgr::clean() {
  // 展開済画像を全削除
  for (std::list<decoded_image*>::iterator itr = img_list_.begin();
       itr != img_list_.end();
       ) {
    img_map_.erase((*itr)->id);
    if ((*itr)->data) free((*itr)->data);
    if ((*itr)->pal) free((*itr)->pal);
    delete *itr;
    itr = img_list_.erase(itr);
  }

  // テクスチャを全削除
  for (std::list<d3d_texture*>::iterator itr = tex_list_.begin();
       itr != tex_list_.end();
       ) {
    tex_map_.erase((*itr)->id);
    if ((*itr)->data) (*itr)->data->Release();
    delete *itr;
    itr = tex_list_.erase(itr);
  }

  // ムービー用テクスチャを削除
  if (movie_tex_) {
    movie_tex_->data->Release();
    delete movie_tex_;
  }

  if (d3d_vertex_buf_) d3d_vertex_buf_->Release();
  if (d3d_index_buf_) d3d_index_buf_->Release();
  if (d3d_dev_) d3d_dev_->Release();
  if (d3d_) d3d_->Release();
}

RenderMode GraphicMgr::getRenderMode(u32 p_type, u32 p_gs_alpha_pattern) {
  switch (p_type) {
    case 0:    // 通常
      // p_gs_alpha_patternに依存する
      switch (p_gs_alpha_pattern) {
        case 0x44:  // dest.rgb * (1 - tex.a) + tex.rgb * tex.a
          return RenderMode::kBlend;
        case 0x48:  // dest.rgb + tex.rgb * tex.a
          return RenderMode::kAdd;
        case 0x26:  // ~dest.rgb
          return RenderMode::kInvDest;
      }
      break;
    case 1:         // tex.rgb
      return RenderMode::kSrcCopy;
    case 3:         // dest.rgb + color2.rgb
    case 4:         // dest.rgb + color1.rgb * color1.a
      return RenderMode::kAddColor;
    default:
      break;
  }
  assert(0); // unexpected render mode
  return RenderMode::kBlend;
}

void GraphicMgr::drawTriangleList(u32 p_vertex_count,
                        u8* p_data,
                        u32 p_img_idx,
                        u32 p_type) {
  HRESULT hr;
  D3DVertex *vbuf;
  
  if (p_vertex_count <= 0) return;

  u16* ibuf;
  hr = d3d_index_buf_->Lock(0, 0, (void**)&ibuf, 0);
  assert(SUCCEEDED(hr));

  int index_count = 0;

  for (u16 i = 0; i < p_vertex_count - 2; i++) {
    if ((i & 1) == 0) {
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 0;
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 1;
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 2;
    } else {
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 0;
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 2;
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 1;
    }
  }
  
  hr = d3d_index_buf_->Unlock();
  assert(SUCCEEDED(hr));

  hr = d3d_vertex_buf_->Lock(0, 0, (void**)&vbuf, 0);
  assert(SUCCEEDED(hr));

  ImageRefData* imgRefData = g_dev_emu->ps2_img_ref_data(p_img_idx);

  float rw = imgRefData->tex_w / (float)(1 << imgRefData->tex_w_bits);
  float rh = imgRefData->tex_h / (float)(1 << imgRefData->tex_h_bits);
  float spriteZ = 0.0f;
  for (u32 i = 0; i < p_vertex_count; i++) {
    float x, y, z, u, v;
    u32   color1, color2;
    if (p_img_idx != 0xffffffff && imgRefData->encoded_data) {
      imgRefData->request++;
      struct VertexDataWithTex {
        float x;
        float y;
        float z;
        float rhw;
        float u;
        float v;
        u32   color1;
        u32   color2;
      } *vertexData = (VertexDataWithTex*)p_data;
      x = vertexData[i].x;
      y = vertexData[i].y;
      z = vertexData[i].z;
      u = vertexData[i].u * rw;
      v = vertexData[i].v * rh;
      color1 = vertexData[i].color1;
      color2 = vertexData[i].color2;
    } else {
      struct VertexDataWithoutTex {
        float x;
        float y;
        float z;
        u32   color;
      } *vertexData = (VertexDataWithoutTex*)p_data;
      x = vertexData[i].x;
      y = vertexData[i].y;
      z = vertexData[i].z;
      u = 0.0f;
      v = 0.0f;
      color1 = vertexData[i].color;
      color2 = 0x00000000;
    }
    vbuf[d3d_vtx_count_ + i].x = floorf(x + 0.5f) - 0.5f;
    vbuf[d3d_vtx_count_ + i].y = floorf(y + 0.5f) - 0.5f;
    vbuf[d3d_vtx_count_ + i].z = z;
    vbuf[d3d_vtx_count_ + i].rhw = 1.0f;
    switch (p_type) {
      case 0:
        vbuf[d3d_vtx_count_ + i].color = color1;
        break;
      case 1:
        vbuf[d3d_vtx_count_ + i].color = color1 | 0xff000000;
        break;
      case 3:
        vbuf[d3d_vtx_count_ + i].color = color2 | 0xff000000;
        break;
      case 4:
        vbuf[d3d_vtx_count_ + i].color = (color1 & 0xff000000) |
                                         (color2 & 0x00ffffff);
        break;
      default: assert(0); break;
    }
    vbuf[d3d_vtx_count_ + i].u = u;
    vbuf[d3d_vtx_count_ + i].v = v;
    spriteZ += z;
  }
  spriteZ /= p_vertex_count;

  hr = d3d_vertex_buf_->Unlock();
  assert(SUCCEEDED(hr));

  // 動的パレットを使用するかどうか…
  u32* dyn_pal_ptr = NULL;
  u32  dyn_pal_ofs = 0;
  if (g_dev_emu->ps2_drawing_boi_va()) {
    int  use_dyn_pal;
    int  chara_id;
    int  pal_ofs;
    int  use_extr_pal;
    u8   player_side;
    bool has_parent;
    if (strcmp(g_elf_name, "SLPS_251.84") == 0) {
      BattleObjectInfo* drawing_boi =
        (BattleObjectInfo*)VA2PA(g_dev_emu->ps2_drawing_boi_va());
      BattleObjectInfo* chara_boi =
        (BattleObjectInfo*)VA2PA(g_dev_emu->ps2_chara_boi_va());
      use_dyn_pal  = drawing_boi->pal_id != 0xff;
      player_side  = drawing_boi->player_side;
      use_extr_pal = drawing_boi->use_extr_pal;
      dyn_pal_ofs  = drawing_boi->extr_pal_ofs * 16;
      has_parent   = drawing_boi->parent_info_ptr != 0;
      chara_id     = chara_boi[player_side].chara_id - 1;
      pal_ofs      = 0;
    } else if (strcmp(g_elf_name, "SLPS_252.61") == 0) {
      BattleObjectInfo* drawing_boi =
        (BattleObjectInfo*)VA2PA(g_dev_emu->ps2_drawing_boi_va());
      BattleObjectInfo* chara_boi =
        (BattleObjectInfo*)VA2PA(g_dev_emu->ps2_chara_boi_va());
      use_dyn_pal  = drawing_boi->pal_id != 0xff;
      player_side  = drawing_boi->player_side;
      use_extr_pal = drawing_boi->use_extr_pal;
      dyn_pal_ofs  = drawing_boi->extr_pal_ofs * 16;
      has_parent   = drawing_boi->parent_info_ptr != 0;
      chara_id     = chara_boi[player_side].chara_id - 1;
      pal_ofs      = 0;
    } else if (strcmp(g_elf_name, "SLPM_663.33") == 0) {
      BattleObjectInfo* drawing_boi =
        (BattleObjectInfo*)VA2PA(g_dev_emu->ps2_drawing_boi_va());
      BattleObjectInfo* chara_boi =
        (BattleObjectInfo*)VA2PA(g_dev_emu->ps2_chara_boi_va());
      use_dyn_pal  = drawing_boi->pal_id != 0xff;
      player_side  = drawing_boi->player_side;
      use_extr_pal = drawing_boi->use_extr_pal;
      dyn_pal_ofs  = drawing_boi->extr_pal_ofs * 16;
      has_parent   = drawing_boi->parent_info_ptr != 0;
      chara_id     = chara_boi[player_side].chara_id - 1;
      pal_ofs      = g_dev_emu->ps2_character_mode(player_side) == 2 ? 0 : 12;
    } else if (strcmp(g_elf_name, "SLPM_669.65") == 0) {
      BattleObjectInfo669_65* drawing_boi =
        (BattleObjectInfo669_65*)VA2PA(g_dev_emu->ps2_drawing_boi_va());
      BattleObjectInfo669_65* chara_boi =
        (BattleObjectInfo669_65*)VA2PA(g_dev_emu->ps2_chara_boi_va());
      use_dyn_pal  = drawing_boi->pal_id != 0xff;
      player_side  = drawing_boi->player_side;
      use_extr_pal = drawing_boi->use_extr_pal;
      dyn_pal_ofs  = drawing_boi->extr_pal_ofs * 16;
      has_parent   = drawing_boi->parent_info_ptr != 0;
      chara_id     = chara_boi[player_side].chara_id - 1;
      pal_ofs      = 0;
    }
    if (use_dyn_pal) {
      int pal_side;
      if (use_extr_pal == 2) {
        pal_side = (player_side + 1) & 1; // 相手のパレットを使用
      } else {
        pal_side = player_side;           // 自分のパレットを使用
      }
      dyn_pal_ptr = g_dev_emu->ps2_dynamic_pal_addr(pal_side);
    } else if (use_extr_pal != 0 && has_parent) {
      if (imgRefData->render_mode == 0x00000420 || // ABAの鍵
          imgRefData->render_mode == 0x00000060) { // エディ分身
        // パレットアニメを適用(キャラと同じパレットを使用)
        dyn_pal_ptr = g_dev_emu->ps2_dynamic_pal_addr(player_side);
      } else {
        // パレットアニメを適用しない
        u32  chara_mode = g_dev_emu->ps2_character_mode(player_side);
        u32* pal_addr = g_dev_emu->getCharaPaletteAddr(chara_mode, chara_id);
        u32  pal_id   = g_dev_emu->ps2_chara_pal_id(player_side) + pal_ofs;
        dyn_pal_ptr   = (u32*)(VA2PA(pal_addr[pal_id]) + 16);
      }
    }
  }

  RenderMode render_mode = getRenderMode(p_type, g_dev_emu->ps2_render_mode());
  if (render_entry_count_ > 0 &&
      render_entry_[render_entry_count_ - 1].img_idx == p_img_idx &&
      render_entry_[render_entry_count_ - 1].z == spriteZ &&
      render_entry_[render_entry_count_ - 1].render_mode == render_mode &&
      render_entry_[render_entry_count_ - 1].dyn_pal_ptr == dyn_pal_ptr &&
      render_entry_[render_entry_count_ - 1].dyn_pal_ofs == dyn_pal_ofs) {
    // 同条件の描画が直前に存在するなら結合
    render_entry_[render_entry_count_ - 1].index_count += index_count;
    render_entry_[render_entry_count_ - 1].vertex_count += p_vertex_count;
  } else {
    // 新規エントリ
    render_entry_[render_entry_count_].idx = render_entry_count_;
    render_entry_[render_entry_count_].img_idx = p_img_idx;
    render_entry_[render_entry_count_].z = spriteZ;
    render_entry_[render_entry_count_].tex = NULL;
    render_entry_[render_entry_count_].start_vertex = d3d_vtx_count_;
    render_entry_[render_entry_count_].start_index = d3d_idx_count_;
    render_entry_[render_entry_count_].vertex_count = p_vertex_count;
    render_entry_[render_entry_count_].index_count = index_count;
    render_entry_[render_entry_count_].render_mode = render_mode;
    render_entry_[render_entry_count_].dyn_pal_ptr = dyn_pal_ptr;
    render_entry_[render_entry_count_].dyn_pal_ofs = dyn_pal_ofs;
    render_entry_count_++;
    //if ((render_entry_count_ % 10) == 0)
    //  DBGOUT_CPU("render_entry_count_=%d\n", render_entry_count_);
  }
  d3d_idx_count_ += index_count;
  d3d_vtx_count_ += p_vertex_count;
}

void GraphicMgr::clearMovieTexture() {
  D3DLOCKED_RECT lockedRect;
  HRESULT hr = movie_tex_->data->LockRect(0, &lockedRect, NULL, D3DLOCK_DISCARD);
  if (FAILED(hr)) {
  }
  for (u32 y = 0; y < kScreenHeight; y++) {
      u32* dp = (u32*)((u8*)lockedRect.pBits + lockedRect.Pitch * y);
      for (u32 x = 0; x < kScreenWidth; x++) {
        dp[x] = 0x00000000;
      }
    }
  hr = movie_tex_->data->UnlockRect(0);
  if (FAILED(hr)) {
  }
}

void GraphicMgr::drawMoviePicture(u32* p_dst_buf_ptr, bool p_has_decoded_image) {
  const int kVertexCount = 4;
  const float kMovieZ = 10000.0f;

  HRESULT hr;

  //ds_util::DBGOUTA("movie draw buf=0x%08x has_img=%x\n",
  //                 p_dst_buf_ptr,
  //                 p_has_decoded_image);

  // 新規にデコードされたピクチャがあれば
  // Movie用のテクスチャを更新する
  if (*p_dst_buf_ptr && p_has_decoded_image) {
    D3DLOCKED_RECT lockedRect;
    hr = movie_tex_->data->LockRect(0, &lockedRect, NULL, D3DLOCK_DISCARD);
    if (FAILED(hr)) {
    }

    u32* movie_dest_buf = (u32*)VA2PA(*p_dst_buf_ptr);

    for (u32 y = 0; y < kScreenHeight; y++) {
      u32* dp = (u32*)((u8*)lockedRect.pBits + lockedRect.Pitch * y);
      u32* sp = movie_dest_buf + (kScreenWidth * y);
      for (u32 x = 0; x < kScreenWidth; x++) {
        dp[x] = sp[x];
      }
    }
    
    hr = movie_tex_->data->UnlockRect(0);
    if (FAILED(hr)) {
    }
  }

  // ムービー描画用の四角ポリゴンを作成
  u16* ibuf;
  hr = d3d_index_buf_->Lock(0, 0, (void**)&ibuf, 0);
  assert(SUCCEEDED(hr));

  int index_count = 0;

  for (int i = 0; i < kVertexCount - 2; i++) {
    if ((i & 1) == 0) {
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 0;
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 1;
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 2;
    } else {
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 0;
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 2;
      ibuf[d3d_idx_count_ + index_count++] = d3d_vtx_count_ + i + 1;
    }
  }
  hr = d3d_index_buf_->Unlock();
  assert(SUCCEEDED(hr));

  D3DVertex *vbuf;
  hr = d3d_vertex_buf_->Lock(0, 0, (void**)&vbuf, 0);
  assert(SUCCEEDED(hr));

  const float rw = (float)movie_tex_->w / movie_tex_->tw;
  const float rh = (float)movie_tex_->h / movie_tex_->th;
  for (int i = 0; i < kVertexCount; i++) {
    vbuf[d3d_vtx_count_ + i].x = (float)((i & 1) ? kScreenWidth : 0) - 0.5f;
    vbuf[d3d_vtx_count_ + i].y = (float)((i & 2) ? kScreenHeight : 0) - 0.5f;
    vbuf[d3d_vtx_count_ + i].z = kMovieZ;
    vbuf[d3d_vtx_count_ + i].rhw = 1.0f;
    vbuf[d3d_vtx_count_ + i].color = 0xffffffff;
    vbuf[d3d_vtx_count_ + i].u = (i & 1) ? rw : 0.0f;
    vbuf[d3d_vtx_count_ + i].v = (i & 2) ? rh : 0.0f;
  }
  hr = d3d_vertex_buf_->Unlock();
  assert(SUCCEEDED(hr));

  // 新規エントリ
  render_entry_[render_entry_count_].idx          = render_entry_count_;
  render_entry_[render_entry_count_].img_idx      = 0xffffffff;
  render_entry_[render_entry_count_].z            = kMovieZ;
  render_entry_[render_entry_count_].tex          = movie_tex_;
  render_entry_[render_entry_count_].start_vertex = d3d_vtx_count_;
  render_entry_[render_entry_count_].start_index  = d3d_idx_count_;
  render_entry_[render_entry_count_].vertex_count = kVertexCount;
  render_entry_[render_entry_count_].index_count  = index_count;
  render_entry_[render_entry_count_].render_mode  = RenderMode::kBlend;
  render_entry_[render_entry_count_].dyn_pal_ptr  = NULL;
  render_entry_[render_entry_count_].dyn_pal_ofs  = 0;
  render_entry_count_++;

  d3d_idx_count_  += index_count;
  d3d_vtx_count_ += kVertexCount;
}

u32 GraphicMgr::getUniqueImg(u32 p_enc_data, u32 p_img_id) {
  return ((p_enc_data & 0x00ffff00) << 8) | (p_img_id & 0xffff);
}

u64 GraphicMgr::getUniqueTex(u32* p_pal_data, u32 p_enc_data, u32 p_img_id) {
  u32 check_sum = 0;
  if (p_pal_data) {
    for (int i = 0; i < 256; i++) {
      check_sum ^= p_pal_data[i] * i;
    }
  }
  return (((u64)check_sum) << 36) |
         (((u64)p_enc_data & 0x0ffffff0) << 12) |
         (p_img_id & 0xffff);
}

void GraphicMgr::decodeAllImages() {

  // かならず更新されるようにする
  //for (std::list<decoded_image*>::iterator itr = img_list_.begin();
  //     itr != img_list_.end();
  //     ) {
  //  img_map_.erase((*itr)->id);
  //  if ((*itr)->data) free((*itr)->data);
  //  if ((*itr)->pal) free((*itr)->pal);
  //  delete *itr;
  //  itr = img_list_.erase(itr);
  //}

  for (u32 i = 0; i < g_dev_emu->ps2_max_img_ref_count(); i++) {
    ImageRefData* imgRefData = g_dev_emu->ps2_img_ref_data(i);

    imgRefData->decoded_data = (u8*)0xffffffff;
    imgRefData->idx = 0xffffffff;

    if (imgRefData->request == 0) continue;
    else imgRefData->request = 0;
    
    u32 img_id =
      getUniqueImg((u32)imgRefData->encoded_data, imgRefData->img_id);

    std::map<u32, decoded_image*>::iterator itr = img_map_.find(img_id);

    if (itr != img_map_.end()) {
      // ある場合はそれ使う
      itr->second->ttl = kDecodedImageTTL;
      imgRefData->decoded_data = itr->second->data;
    } else {
      if (imgRefData->encoded_data == 0) {
        DBGOUT_CPU("requestがあるのにencoded_dataがない！！\n");
        continue;
      } else if (imgRefData->encoded_data == (u8*)0xffffffff) {
        assert(0);
      } else {
        // ない場合はenc_dataから展開する
        int bufsize;
        if (imgRefData->bpp == 4) {
          bufsize = imgRefData->tex_w * imgRefData->tex_h / 2;
        } else if (imgRefData->bpp == 8) {
          bufsize = imgRefData->tex_w * imgRefData->tex_h;
        } else {
          assert(0);
        }

        // 元の展開コードdecode1のバグ(仕様?)で
        // サイズ理論値を僅かに超えて展開される可能性がある。
        // サイズチェックの時間が無駄なので16バイト余分に取っておく
        // ちなみにi == 0x72eで発生
        decoded_image* img = new decoded_image;
        img->id      = img_id;
        img->data    = (u8*)malloc(bufsize + 16);
        if (imgRefData->fixed_pal == 0x20) {
          // パレットを画像内に持っている
          img->pal_size = 1 << imgRefData->bpp;
          img->pal    = (u32*)malloc(img->pal_size * sizeof(u32));
          memcpy(img->pal,
            (u8*)VA2PA((u32)imgRefData->encoded_data + 0x10),
            img->pal_size * sizeof(u32));
        } else if (imgRefData->fixed_pal == 0x00) {
          // パレットを画像内に持っていない
          img->pal_size = 0;
          img->pal    = NULL;
        } else {
          assert(0);
        }

        u8* enc_data_body =
          imgRefData->encoded_data + 0x10 + img->pal_size * sizeof(u32);
        switch (imgRefData->enc_type) {
        case 5:
          decode5(img->data, (u8*)VA2PA((u32)enc_data_body));
          imgRefData->decoded_data = img->data;
          //DBGOUT_CPU("decoded_data生成 enctype=5 id=0x%03x "
          //  "img=0x%05d buf=0x%08x size=0x%x\n",
          //  i, imgRefData->img_id, img->data, bufsize);
          break;
        case 1:
          decode1(img->data, (u8*)VA2PA((u32)enc_data_body));
          imgRefData->decoded_data = img->data;
          //DBGOUT_CPU("decoded_data生成 enctype=1 id=0x%03x "
          //  "img=0x%05d buf=0x%08x size=0x%x\n",
          //  i, imgRefData->img_id, img->data, bufsize);
          break;
        case 0:
          memcpy(img->data, (u8*)VA2PA((u32)enc_data_body), bufsize);
          imgRefData->decoded_data = img->data;
          //DBGOUT_CPU("decoded_data生成 enctype=0 id=0x%03x "
          //  "img=0x%05d buf=0x%08x size=0x%x\n",
          //  i, imgRefData->img_id, img->data, bufsize);
          break;
        default:
          DBGOUT_CPU("想定されていない画像エンコード方式！！\n");
          assert(0);
          break;
        }
        img->ttl = kDecodedImageTTL;
        img->listitr = img_list_.insert(img_list_.begin(), img);
        img_map_.insert(std::pair<u32, decoded_image*>(img_id, img));

#ifdef _EXPORT_IMAGE
        exportImage(imgRefData, img);
#endif
      }
    }
  }

  // 使用していないデータを削除
  for (std::list<decoded_image*>::iterator itr = img_list_.begin(); itr != img_list_.end();) {
    if ((*itr)->ttl <= 0) {
      img_map_.erase((*itr)->id);
      if ((*itr)->data) free((*itr)->data);
      if ((*itr)->pal) free((*itr)->pal);
      delete *itr;
      itr = img_list_.erase(itr);
    } else {
      (*itr)->ttl--;
      itr++;
    }
  }
}

void GraphicMgr::renderStart() {
  rendering_now_ = true;

  u32 color = ((g_dev_emu->ps2_clear_color() << 16) & 0xff0000) | // R
              ((g_dev_emu->ps2_clear_color())       & 0xff00) |   // G
              ((g_dev_emu->ps2_clear_color() >> 16) & 0xff);      // B
  d3d_dev_->Clear(0, NULL, D3DCLEAR_TARGET, color, 1.0f, 0);
  d3d_dev_->BeginScene();
}

int GraphicMgr::compareRenderEntryZ(const RenderEntry* p_a, const RenderEntry* p_b) {
  float diff = p_b->z - p_a->z;

  if (diff < 0.0f) return -1;
  else if (diff > 0.0f) return 1;
  else {
    // zが同じなら描画要求順を維持する
    return p_a->idx - p_b->idx;
  }
}

d3d_texture* GraphicMgr::getTexture(u64 p_id) {
  std::map<u64, d3d_texture*>::iterator itr = tex_map_.find(p_id);
  return (itr != tex_map_.end()) ? itr->second : NULL;
}

d3d_texture* GraphicMgr::createTexture(u64 p_id, u8* p_data, u32 p_tw, u32 p_th, u32 p_w, u32 p_h, int p_bpp, u32* p_pal) {
  
  // テクスチャを新規作成し、リスト、マップに登録する
  d3d_texture* tex = new d3d_texture;
  tex->id  = p_id;
  tex->ttl = kD3DTextureTTL;
  tex->w   = p_w;
  tex->h   = p_h;
  tex->tw  = p_tw;
  tex->th  = p_th;
  HRESULT hr = d3d_dev_->CreateTexture(p_tw, p_th, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex->data, NULL);
  if (FAILED(hr)) throw "IDirect3DDevice9::CreateTexture()に失敗";

  D3DLOCKED_RECT lockedRect;
  hr = tex->data->LockRect(0, &lockedRect, NULL, D3DLOCK_DISCARD);
  if (FAILED(hr)) throw "IDirect3DTexture9::LockRect()に失敗";
  
  if (p_bpp == 4) {
    for (u32 y = 0; y < p_h; y++) {
      u32* dp = (u32*)((u8*)lockedRect.pBits + lockedRect.Pitch * y);
      u8*  sp = p_data + (p_w * y >> 1);
      for (u32 x = 0; x < p_w >> 1; x++) {
        *(dp + (x << 1))     = p_pal[*(sp + x) & 0x0f];
        *(dp + (x << 1) + 1) = p_pal[*(sp + x) >> 4];
      }
    }
  } else if (p_bpp == 8) {
    for (u32 y = 0; y < p_h; y++) {
      u32* dp = (u32*)((u8*)lockedRect.pBits + lockedRect.Pitch * y);
      u8*  sp = p_data + (p_w * y);
      for (u32 x = 0; x < p_w; x++) {
        *(dp + x) = p_pal[*(sp + x)];
      }
    }
  }
  
  hr = tex->data->UnlockRect(0);
  if (FAILED(hr)) throw "IDirect3DTexture9::UnlockRect()に失敗";

  tex->listitr = tex_list_.insert(tex_list_.begin(), tex);
  tex_map_.insert(std::pair<u64, d3d_texture*>(p_id, tex));
  
  return tex;
}

void GraphicMgr::renderEnd() {

  // zでソート
  qsort(render_entry_, render_entry_count_, sizeof(RenderEntry), (int(*)(const void*, const void*))GraphicMgr::compareRenderEntryZ);

  bool last_alpha_enable = TRUE;
  bool last_modulate = TRUE;
  for (int i = 0; i < render_entry_count_; i++) {
    if (render_entry_[i].index_count >= 3) {
      if (render_entry_[i].tex) {
        // テクスチャ指定ありならそれを使用。実質Movie用にのみ専用のテクスチャが指定される
        d3d_dev_->SetTexture(0, render_entry_[i].tex->data);
      } else {
        if (render_entry_[i].img_idx == 0xffffffff) {
          // テクスチャなし
          d3d_dev_->SetTexture(0, NULL);
        } else {
          ImageRefData* ref_data = g_dev_emu->ps2_img_ref_data(render_entry_[i].img_idx);

          u64 tex_id = getUniqueTex(render_entry_[i].dyn_pal_ptr, (u32)ref_data->encoded_data, ref_data->img_id);

          d3d_texture* tex = getTexture(tex_id);
          if (tex) {
            // ある場合は再利用しTTLをリセット
            tex->ttl = kDecodedImageTTL;
          } else {
            // 無い場合はDecodedDataから再構築する
            u32 img_id = getUniqueImg((u32)ref_data->encoded_data, ref_data->img_id);
            std::map<u32, decoded_image*>::iterator itr = img_map_.find(img_id);
            if (itr == img_map_.end()) {
              continue;
            }

            // 使用するパレットを選択する
            u32* gspal;
            int pal_count;
            if (render_entry_[i].dyn_pal_ptr) {
              gspal = render_entry_[i].dyn_pal_ptr;
              pal_count = 256;
            } else {
              gspal = itr->second->pal;
              pal_count = 1 << ref_data->bpp;
            }
            assert(gspal);

            // GS用のパレットをD3D用に変換
            u32 pal[256];
            for (int j = 0; j < pal_count; j++) {
              int k = j;
              if (pal_count == 256) {
                k = (k & 0xe7) | ((k & 0x08) << 1) | ((k & 0x10) >> 1);
              }
              u32 alpha = gspal[k] >> 24;
              if (j > 0 && j == render_entry_[i].dyn_pal_ofs) {
                // 強制的に透明色
                alpha = 0;
              }
              u32 color = ((gspal[k] << 16) & 0xff0000) | (gspal[k] & 0xff00) | ((gspal[k] >> 16) & 0xff);
              if (alpha >= 0x80) {
                pal[j] = 0xff000000 | color;
              } else {
                pal[j] = (alpha << 25) | color;
              }
            }

            tex = createTexture(
              tex_id,
              ref_data->decoded_data,
              1 << ref_data->tex_w_bits,
              1 << ref_data->tex_h_bits,
              ref_data->tex_w,
              ref_data->tex_h,
              ref_data->bpp,
              pal + render_entry_[i].dyn_pal_ofs);
            //DBGOUT_CPU("d3d_texture生成 id=0x%08x%08x decimg=0x%08x\n",
            //           (u32)(tex_id >> 32), (u32)(tex_id), ref_data->decoded_data);
          }
          d3d_dev_->SetTexture(0, tex->data);
        }
      }

      bool alpha = TRUE;
      bool modulate = TRUE;
      switch (render_entry_[i].render_mode) {
        case RenderMode::kBlend:    // 通常or半透明
          d3d_dev_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
          d3d_dev_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
          break;
        case RenderMode::kAdd:      // 加算合成
          d3d_dev_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
          d3d_dev_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
          break;
        case RenderMode::kInvDest:  // 絶命拳
          d3d_dev_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
          d3d_dev_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
          break;
        case RenderMode::kSrcCopy:  // 背景など
          // 透過処理不要と思ったが一部(SlashのHeavenOrHell)で
          // アルファブレンド必須のためkBlendと同じ動作とした
          //d3d_dev_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
          //d3d_dev_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
          //alpha = FALSE;
          d3d_dev_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
          d3d_dev_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
          break;
        case RenderMode::kAddColor: // エディステージ、一撃エフェクト(Slash)
          d3d_dev_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
          d3d_dev_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
          modulate = FALSE;
          break;
      }

      if (alpha != last_alpha_enable) {
        d3d_dev_->SetRenderState(D3DRS_ALPHABLENDENABLE, alpha);
        last_alpha_enable = alpha;
      }
      if (modulate != last_modulate) {
        d3d_dev_->SetTextureStageState(0, D3DTSS_COLOROP,
          modulate ? D3DTOP_MODULATE : D3DTOP_SELECTARG1);
        last_modulate = modulate;
      }

      HRESULT hr = d3d_dev_->DrawIndexedPrimitive(
        D3DPT_TRIANGLELIST,
        0,
        render_entry_[i].start_vertex,
        render_entry_[i].vertex_count,
        render_entry_[i].start_index,
        render_entry_[i].index_count / 3);
      assert(SUCCEEDED(hr));
    }
  }
  // 後始末
  if (last_alpha_enable == FALSE) {
    d3d_dev_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  }
  if (last_modulate == FALSE) {
    d3d_dev_->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  }

  // 使用していないデータを削除
  for (std::list<d3d_texture*>::iterator itr = tex_list_.begin();
       itr != tex_list_.end();
       ) {
    if ((*itr)->ttl <= 0) {
      tex_map_.erase((*itr)->id);
      if ((*itr)->data) (*itr)->data->Release();
      delete *itr;
      itr = tex_list_.erase(itr);
    } else {
      (*itr)->ttl--;
      itr++;
    }
  }

  render_entry_count_ = 0;
  d3d_vtx_count_ = 0;
  d3d_idx_count_ = 0;

  d3d_dev_->EndScene();
  HRESULT hr;
#ifdef _DEBUGGER
  hr = d3d_dev_->Present(NULL, NULL, NULL, NULL);
#else
  if (g_app.setting_file_mgr()->get_data()->graph_fullscreen() ==
      SettingFileData::kDisplayMode_FullScreenExcl) {
    // 排他モード
    hr = d3d_dev_->Present(NULL, NULL, NULL, NULL);
  } else {
    RECT src_bounds = { 0, 0, kScreenWidth, kScreenHeight };
    RECT dest_bounds;
    if (full_screen_mode_ &&
        g_app.setting_file_mgr()->get_data()->graph_keep_aspect_ratio()) {
      u32 size, margin;
      static const double req_ratio = (double)kScreenWidth / kScreenHeight;
      if (physical_screen_ratio_ > req_ratio) {
        size = (u32)(d3dpp_full_.BackBufferWidth  * (req_ratio / physical_screen_ratio_));
        margin = (d3dpp_full_.BackBufferWidth - size) / 2;
        dest_bounds.left   = margin;
        dest_bounds.top    = 0;
        dest_bounds.right  = margin + size;
        dest_bounds.bottom = d3dpp_full_.BackBufferHeight;
      } else {
        size = (u32)(d3dpp_full_.BackBufferHeight * (req_ratio / physical_screen_ratio_));
        margin = (d3dpp_full_.BackBufferHeight - size) / 2;
        dest_bounds.left   = 0;
        dest_bounds.top    = margin;
        dest_bounds.right  = d3dpp_full_.BackBufferHeight;
        dest_bounds.bottom = margin + size;
      }
    } else {
      dest_bounds = src_bounds;
    }
    hr = d3d_dev_->Present(&src_bounds, &dest_bounds, NULL, NULL);
  }
#endif
  if (hr == D3DERR_DEVICELOST) {
    device_lost_ = true;
  }
  rendering_now_ = false;
}

#if !defined _DEBUGGER
void GraphicMgr::restoreDevice(void) {

  if (device_lost_) {
    // レンダリング中なら終わるまで待機しないと落ちる
    while (rendering_now_) {
      Sleep(1);
    }

    HRESULT hr = d3d_dev_->TestCooperativeLevel();
    if (FAILED(hr)) {
      if (hr != D3DERR_DEVICENOTRESET) return;

      SuspendThread(g_execute_thread_hdl);

      if (g_app.setting_file_mgr()->get_data()->graph_fullscreen() == SettingFileData::kDisplayMode_FullScreenExcl) {
        hr = d3d_dev_->Reset(full_screen_mode_ ? &d3dpp_full_ : &d3dpp_window_);
      } else {
        hr = d3d_dev_->Reset(&d3dpp_window_);
      }
      if (FAILED(hr)) {
        if (hr == D3DERR_DEVICELOST) return;
        DBGOUT_APP("Failed to resume Direct3D device.");
      } else {
        setup();
        ResumeThread(g_execute_thread_hdl);
        device_lost_ = false;
      }
    }
  }
}

BOOL CALLBACK GraphicMgr::enumMonitorCallback(HMONITOR p_hmon,
                                    HDC p_hdc,
                                    LPRECT p_rect,
                                    LPARAM p_data) {
  MONITORINFOEX* minfo = (MONITORINFOEX*)p_data;
  memset(minfo, 0x00, sizeof(MONITORINFOEX));
  minfo->cbSize = sizeof(MONITORINFOEX);
  GetMonitorInfo(p_hmon, minfo);
  return strcmp(g_app.setting_file_mgr()->get_data()->graph_display_device(), minfo->szDevice) != 0;
}

void GraphicMgr::toggleScreenMode() {
  // レンダリング中なら終わるまで待機しないと落ちる
  while (rendering_now_) {
    Sleep(1);
  }

  SuspendThread(g_execute_thread_hdl);

  if (full_screen_mode_ == false) {
    // style
    save_window_style_ = GetWindowLong(g_scrn->hwnd(), GWL_STYLE);
    // size & pos
    GetWindowRect(g_scrn->hwnd(), &save_window_rect_);
  }

  full_screen_mode_ = !full_screen_mode_;

  if (g_app.setting_file_mgr()->get_data()->graph_fullscreen() == SettingFileData::kDisplayMode_FullScreenExcl) {
    HRESULT hr = d3d_dev_->Reset(full_screen_mode_ ? &d3dpp_full_ : &d3dpp_window_);
    if (FAILED(hr)) {
      if (hr == D3DERR_DEVICELOST) {
        device_lost_ = true;
        return;
      }
      DBGOUT_APP("Failed to switch screen mode.");
    }
    setup();
  } else {
    if (full_screen_mode_) {
      //DISPLAY_DEVICE ddinfo;
      //memset(&ddinfo, 0x00, sizeof(DISPLAY_DEVICE));
      //ddinfo.cb = sizeof(DISPLAY_DEVICE);
      //int i = 0;
      //while (1) {
      //  if (!EnumDisplayDevices(NULL, i++, &ddinfo, 0)) break;
      //}
      MONITORINFOEX target_monitor;
      EnumDisplayMonitors(NULL, NULL, GraphicMgr::enumMonitorCallback, (LPARAM)&target_monitor);

      DEVMODE devmode;
      memset(&devmode, 0, sizeof(DEVMODE));
      devmode.dmSize = sizeof(DEVMODE);
      devmode.dmPelsWidth = d3dpp_full_.BackBufferWidth;
      devmode.dmPelsHeight = d3dpp_full_.BackBufferHeight;
      devmode.dmBitsPerPel = g_app.setting_file_mgr()->get_data()->graph_color_depth();
      devmode.dmDisplayFrequency = 60;
      devmode.dmFields = DM_BITSPERPEL |
                         DM_PELSWIDTH |
                         DM_PELSHEIGHT |
                         DM_DISPLAYFREQUENCY |
                         DM_DISPLAYFLAGS;
      ChangeDisplaySettingsEx(target_monitor.szDevice, &devmode, NULL, CDS_FULLSCREEN, NULL);

      EnumDisplayMonitors(NULL, NULL, enumMonitorCallback, (LPARAM)&target_monitor);

      // style
      SetWindowLong(g_scrn->hwnd(), GWL_STYLE,
                    WS_POPUP | WS_VISIBLE);
      // size & pos
      SetWindowPos(g_scrn->hwnd(), HWND_TOPMOST,
                   target_monitor.rcMonitor.left,
                   target_monitor.rcMonitor.top,
                   d3dpp_full_.BackBufferWidth,
                   d3dpp_full_.BackBufferHeight,
                   0);
    } else {
      ChangeDisplaySettings(NULL, CDS_FULLSCREEN);
    }
  }

  if (full_screen_mode_ == false) {
    // style
    SetWindowLong(g_scrn->hwnd(), GWL_STYLE,
                  save_window_style_);
    // size & pos
    SetWindowPos(g_scrn->hwnd(), HWND_NOTOPMOST,
                 save_window_rect_.left,
                 save_window_rect_.top,
                 save_window_rect_.right - save_window_rect_.left,
                 save_window_rect_.bottom - save_window_rect_.top,
                 0);
  }

  ResumeThread(g_execute_thread_hdl);
}

  // ウインドウモードに戻してから終了する
#if !defined _DEBUGGER
void GraphicMgr::changeWindowMode() {
  if (full_screen_mode_) {
    g_dev_emu->toggleScreenMode();
  }
}
#endif // defined _DEBUGGER

#endif // !defined _DEBUGGER

bool GraphicMgr::isRenderingNow() {
  return rendering_now_;
}

#endif // defined _CONFIG

void GraphicMgr::decode1(u8* p_dst, u8* p_src) {
  u16* sp = (u16*)p_src;
  u8*  dp = (u8*)p_dst;
  
  u8*  bdp = dp;

  u32 size = *sp << 16 | *(sp + 1);
  if (size <= 0) return;
  sp += 2;

  int cnt = 0;
  for (u32 i = 0; i < size; i++) {
    s32 data = (s32)(*sp << 16 | *(sp + 1));

    cnt = (cnt - 1) & 0xf;
    sp += cnt == 0 ? 2 : 1;

    data >>= cnt;
    if (data & 0x10000) {
      *(bdp++) = (u8)(data >> 8);
      *(bdp++) = (u8)(data);
    } else {
      u32 sz  = (data & 0x7f) + 3;
      u8* sdp = dp + ((data >> 7) & 0x01ff);
      u8* tdp = bdp;
      u32 tsz = sz;
      if (sz != 0) {
        while ((u32)tdp & 3) {
          *(tdp++) = *(sdp++);
          if (tsz-- == 0) break;
        }
      }

      if (((u32)sdp & 1) == 0) {
        if (((u32)sdp & 2) == 0) {
          while (tsz >= 4) {
            *((u32*)tdp) = *((u32*)sdp);
            tdp += 4;
            sdp += 4;
            tsz -= 4;
          }
        } else {
          if (tsz >= 2) {
            *((u16*)tdp) = *((u16*)sdp);
            tdp += 2;
            sdp += 2;
            tsz -= 2;
          }
        }
      }
      while (tsz != 0) {
        *tdp++ = *sdp++;
        tsz--;
      }
      bdp += sz;
    }
    int dstcnt = bdp - dp;
    if (dstcnt >= 0x200) {
      dp = bdp - 0x200;
    }
  }
}

void GraphicMgr::decode5(u8* p_dst, u8* p_src) {
#define FETCH Fetch::fetchBits
  class Fetch {
  public:
    static u32 fetchBits(u32 p_bits, u32 p_counter_init,
        u32 &p_counter, u32 &p_data1, u32 &p_data2, u32* &p_buf) {
      if (p_counter == 0) {
        p_data1 = *p_buf++;
        p_data2 = *p_buf++;
        p_counter = p_counter_init;
      }
      u32 result = p_data1 & ((1 << p_bits) - 1);
      p_data1 = (p_data1 >> p_bits) | (p_data2 << (32 - p_bits));
      p_data2 >>= p_bits;
      p_counter--;
      return result;
    }
  };

  const u32 bits[4] = { 1, 4, 5, 8 };
  const u32 ctr0[4] = { 64, 16, 12, 8 };
  u32 ctr[4]   = { 0, 0, 0, 0 };
  u32 data1[4] = { 0, 0, 0, 0 };
  u32 data2[4] = { 0, 0, 0, 0 };
  u32* buf[4] = {
    (u32*)(p_src + *((u16*)(p_src + 0x08)) * 8),
    (u32*)(p_src + *((u16*)(p_src + 0x0a)) * 8),
    (u32*)(p_src + *((u16*)(p_src + 0x0c)) * 8),
    (u32*)(p_src + *((u16*)(p_src + 0x0e)) * 8),
  };
  
  int idx, block_size, shift_amount;
  if (*((u16*)(p_src + 6)) == 4) {
    idx = 1;
    block_size = *((u16*)(p_src)) / 8;
    shift_amount = 4;
  } else {
    idx = 2;
    block_size = *((u16*)(p_src)) / 4;
    shift_amount = 8;
  }

  u32  *dp = (u32*)p_dst;
  u32  dbuf[4][2] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  int repeat = 0;
  for (int i = *((u16*)(p_src + 0x02)) / 2 - 1; i >= 0; i--) {
    for (int j = *((u16*)(p_src)) / 2 - 1; j >= 0; j--) {
      if (repeat != 0) {
        repeat--;
      } else {
        int data;
        data = (FETCH(bits[0], ctr0[0], ctr[0],
                      data1[0], data2[0], buf[0])) |
               (FETCH(bits[0], ctr0[0], ctr[0],
                      data1[0], data2[0], buf[0]) << 1);

        if (data == 0) {
          data = FETCH(bits[0], ctr0[0], ctr[0],
                             data1[0], data2[0], buf[0]);
          if (data) {
            dbuf[0][0] = FETCH(bits[idx], ctr0[idx], ctr[idx],
                               data1[idx], data2[idx], buf[idx]);
          }
          data = FETCH(bits[0], ctr0[0], ctr[0],
                       data1[0], data2[0], buf[0]);
          if (data) {
            dbuf[0][1] = FETCH(bits[idx], ctr0[idx], ctr[idx],
                               data1[idx], data2[idx], buf[idx]);
          }

          data = FETCH(4, ctr0[1], ctr[1],
                       data1[1], data2[1], buf[1]);

          dbuf[3][1] = ((dbuf[0][(data >> 2) & 1] << shift_amount) | dbuf[0][data >> 3      ]) << (32 - shift_amount * 2);
          dbuf[3][0] = ((dbuf[0][data        & 1] << shift_amount) | dbuf[0][(data >> 1) & 1]) << (32 - shift_amount * 2);

        } else if (data == 1) {
          data = FETCH(bits[0], ctr0[0], ctr[0], data1[0], data2[0], buf[0]);
          if (data) {
            repeat = FETCH(bits[3], ctr0[3], ctr[3], data1[3], data2[3], buf[3]) + 3;
          }
        } else if (data == 2) {
          data = FETCH(bits[0], ctr0[0], ctr[0], data1[0], data2[0], buf[0]);
          if (data) {
            dbuf[0][0] = FETCH(bits[idx], ctr0[idx], ctr[idx], 
                               data1[idx], data2[idx], buf[idx]);
            dbuf[3][0] = (dbuf[0][0] | (dbuf[0][0] << shift_amount)) <<
              (32 - shift_amount * 2);
          } else {
            data = FETCH(bits[0], ctr0[0], ctr[0],
                         data1[0], data2[0], buf[0]);
            dbuf[3][0] = (dbuf[0][data] | (dbuf[0][data] << shift_amount)) <<
              (32 - shift_amount * 2);
          }
          dbuf[3][1] = dbuf[3][0];
        } else if (data == 3) {
          dbuf[3][1]  = FETCH(bits[idx], ctr0[idx], ctr[idx],
                              data1[idx], data2[idx], buf[idx]) << (32 - shift_amount * 2);
          dbuf[3][1] |= FETCH(bits[idx], ctr0[idx], ctr[idx],
                              data1[idx], data2[idx], buf[idx]) << (32 - shift_amount);
          dbuf[3][0]  = FETCH(bits[idx], ctr0[idx], ctr[idx],
                              data1[idx], data2[idx], buf[idx]) << (32 - shift_amount * 2);
          dbuf[3][0] |= FETCH(bits[idx], ctr0[idx], ctr[idx],
                              data1[idx], data2[idx], buf[idx]) << (32 - shift_amount);
          dbuf[0][1] = dbuf[3][0] >> (32 - shift_amount);
          dbuf[0][0] = dbuf[3][1] >> (32 - shift_amount);
        }
      }
      dbuf[2][1] |= dbuf[3][1];
      dbuf[1][1] |= dbuf[3][0];

      if ((j & (11 - shift_amount)) == 0) {
        *(dp)          = dbuf[2][0];
        *(dp + 1)        = dbuf[2][1];
        *(dp + block_size)    = dbuf[1][0];
        *(dp + block_size + 1)  = dbuf[1][1];
        dp += 2;

        dbuf[1][0] = 0;
        dbuf[1][1] = 0;
        dbuf[2][0] = 0;
        dbuf[2][1] = 0;
      } else {
        dbuf[1][0] = (dbuf[1][0] >> (shift_amount * 2)) | (dbuf[1][1] << (32 - shift_amount * 2));
        dbuf[1][1] >>= (shift_amount * 2);
        dbuf[2][0] = (dbuf[2][0] >> (shift_amount * 2)) | (dbuf[2][1] << (32 - shift_amount * 2));
        dbuf[2][1] >>= (shift_amount * 2);
      }
    }
    dp += block_size;
  }
#undef FETCH
}

#ifdef _EXPORT_IMAGE
void exportImage(ImageRefData* p_imgRefData, decoded_image* p_img) {
  u32 key = (u32)p_imgRefData->img_id + (u32)p_imgRefData->encoded_data;
  if (g_saved_image.find(key) == g_saved_image.end()) {
    ds_image dsimage;
    int pal_size = 1 << p_imgRefData->bpp;
    dsimage.create(p_imgRefData->tex_w, p_imgRefData->tex_h, pal_size);
    if (p_img->pal) {
      u32 p[256];
      for (int j = 0; j < p_img->pal_size; j++) {
        int k = j;
        if (p_imgRefData->bpp == 8) {
          k = (k & 0xe7) | ((k & 0x08) << 1) | ((k & 0x10) >> 1);
        }
        p[j] = ((p_img->pal[k] >> 16) & 0xff) |
               (p_img->pal[k] & 0xff00) |
               ((p_img->pal[k] << 16) & 0xff0000);
      }
      dsimage.replacePalette(p, p_img->pal_size);
    } else {
      u32 p[256];
      for (int j = 0; j < 256; j++) {
        p[j] = 0xffffffff;
      }
      p[0] = 0x0000000;
      dsimage.replacePalette(p, 256);
    }
    BYTE* data = dsimage.getData();
    if (p_imgRefData->bpp == 4) {
      for (int j = 0; j < p_imgRefData->tex_h; j++) {
        for (int k = 0; k < p_imgRefData->tex_w >> 1; k++) {
          int idx1 = j * p_imgRefData->tex_w + k * 2;
          int idx2 = j * (p_imgRefData->tex_w >> 1) + k;
          data[idx1]     = p_img->data[idx2] & 0xf;
          data[idx1 + 1] = p_img->data[idx2] >> 4;
        }
      }
    } else if (p_imgRefData->bpp == 8) {
      for (int j = 0; j < p_imgRefData->tex_h; j++) {
        for (int k = 0; k < p_imgRefData->tex_w; k++) {
          int idx1 = j * p_imgRefData->tex_w + k;
          int idx2 = j * p_imgRefData->tex_w + k;
          data[idx1] = p_img->data[idx2];
        }
      }
    }
    s8 str[1024];
    sprintf(str, "%04x_%07x.bmp",
      (u32)p_imgRefData->img_id,
      (u32)p_imgRefData->encoded_data);
    dsimage.save(str);
    g_saved_image.insert(hash_map<int, int>::value_type(key, 0));
  }
}
#endif
