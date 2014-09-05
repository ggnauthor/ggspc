#ifndef _ggsext_graphic
#define _ggsext_graphic

#include "main.h"

#include <map>
#include <list>

#include <d3d9.h>

namespace {

const int kScreenWidth  = 640;
const int kScreenHeight = 480;
const int kMaxVertexBufferEntry = 65536;
const int kMaxIndexBufferEntry  = 65536 * 3;
const int kDecodedImageTTL = 256;
const int kD3DTextureTTL   = 256;
const int kD3DVertexFVF = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);

enum RenderMode {
  kBlend = 0,
  kAdd,
  kInvDest,
  kSrcCopy,
  kAddColor,
};

struct d3d_texture {
  u64 id;
  int ttl;
  std::list<d3d_texture*>::iterator listitr;

  u32 w;
  u32 h;
  u32 tw;
  u32 th;
  LPDIRECT3DTEXTURE9 data;
};

struct decoded_image {
  u32  id;
  int  ttl;
  std::list<decoded_image*>::iterator listitr;

  u8*  data;
  int  pal_size;
  u32* pal;
};

struct D3DVertex {
  float x;
  float y;
  float z;
  float rhw;
  u32   color;
  float u;
  float v;
};

struct RenderEntry {
  u32          idx;
  u32          start_vertex;
  u32          vertex_count;
  u32          start_index;
  u32          index_count;
  float        z;
  u32          img_idx;
  d3d_texture* tex;
  RenderMode   render_mode;
  u32*         dyn_pal_ptr;
  u32          dyn_pal_ofs;
};

}

class GraphicMgr {
public:
#if !defined _CONFIG
  GraphicMgr();
  ~GraphicMgr();

  bool init();
  void setup();
  void clean();

  RenderMode getRenderMode(u32 p_type, u32 p_gs_alpha_pattern);
  void drawTriangleList(u32 p_vertex_count, u8* p_data, u32 p_img_idx, u32 p_type);

  void clearMovieTexture();
  void drawMoviePicture(u32* p_dst_buf_ptr, bool p_has_decoded_image);
  u32  getUniqueImg(u32 p_enc_data, u32 p_img_id);
  u64  getUniqueTex(u32* p_pal_data, u32 p_enc_data, u32 p_img_id);

  void decodeAllImages();
  void renderStart();

  struct d3d_texture* getTexture(u64 p_id);
  struct d3d_texture* createTexture(u64 p_id, u8* p_data, u32 p_tw, u32 p_th, u32 p_w, u32 p_h, int p_bpp, u32* p_pal);
  void renderEnd();
#if !defined _DEBUGGER
  void restoreDevice();
  static BOOL CALLBACK enumMonitorCallback(HMONITOR p_hmon, HDC p_hdc, LPRECT p_rect, LPARAM p_data);
  void toggleScreenMode();
  void changeWindowMode();
#endif // !defined _DEBUGGER
  bool isRenderingNow();
#endif // !defined _CONFIG

  static void decode1(u8* p_dst, u8* p_src);
  static void decode5(u8* p_dst, u8* p_src);

private:
  static int compareRenderEntryZ(const RenderEntry* p_a, const RenderEntry* p_b);

  LPDIRECT3D9           d3d_;
  LPDIRECT3DDEVICE9     d3d_dev_;
  D3DDISPLAYMODE        d3d_display_mode_;
  D3DPRESENT_PARAMETERS d3dpp_window_;
  D3DPRESENT_PARAMETERS d3dpp_full_;
  
  double physical_screen_ratio_;

  volatile bool rendering_now_;
  volatile bool device_lost_;
  bool          full_screen_mode_;

  int  save_window_style_;
  RECT save_window_rect_;

  LPDIRECT3DVERTEXBUFFER9 d3d_vertex_buf_;
  LPDIRECT3DINDEXBUFFER9  d3d_index_buf_;
  int d3d_vtx_count_;
  int d3d_idx_count_;

  std::list<d3d_texture*>       tex_list_;
  std::map<u64, d3d_texture*>   tex_map_;

  std::list<decoded_image*>     img_list_;
  std::map<u32, decoded_image*> img_map_;

  RenderEntry render_entry_[1024];
  int         render_entry_count_;

  d3d_texture* movie_tex_;
};

#endif // _ggsext_graphic
