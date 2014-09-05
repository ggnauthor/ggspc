#include "main.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/source_info.h"
#include "debugger/expr_parser.h"
#include "debugger/analyze_info.h"
#include "debugger/patch_entry.h"
#include "debugger/gui/window/screen_window.h"
#include "debugger/gui/window/register_window.h"
#include "debugger/gui/window/code_window.h"
#include "debugger/gui/window/memory_window.h"
#include "debugger/gui/window/breakpoint_window.h"
#include "debugger/gui/window/trace_window.h"
#include "debugger/gui/window/reference_window.h"
#include "debugger/gui/window/patch_window.h"
#include "debugger/gui/window/profiler_window.h"
#include "debugger/gui/dialog/new_ddb_dialog.h"
#include "debugger/gui/dialog/addr_jump_dialog.h"

#include <algorithm>
#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

Debugger* g_dbg = NULL;

const TypeDefinition Debugger::TYPE_DEF[] = {
  "hex",      0, false,
  "int8",     1, false,
  "uint8",    1, false,
  "int16",    2, false,
  "uint16",   2, false,
  "int32",    4, false,
  "uint32",   4, false,
  "int64",    8, false,
  "uint64",   8, false,
  "float",    4, false,
  "double",   8, false,
  "string",   0, false,
  "pointer",  4,  true,
  "",        -1, false,
};

unsigned int hash_int(int& p_key) {
  return p_key;
}

Debugger::Debugger() {
  InitCommonControls();

  register_window_ = new RegisterWindow();
  code_window_ = new CodeWindow();
  memory_window_ = new MemoryWindow();
  breakpoint_window_ = new BreakPointWindow();
  trace_window_ = new TraceWindow();
  reference_window_ = new ReferenceWindow();
  patch_window_ = new PatchWindow();
  profiler_window_ = new ProfilerWindow();

  actual_size_screen_ = false;

  enable_log_cpu_     = false;
  enable_log_gs_      = false;
  enable_log_mem_     = false;
  enable_log_elf_     = false;
  enable_log_dma_     = false;
  enable_log_rec_     = false;
  enable_log_app_     = false;
  enable_log_ddb_     = false;
  enable_log_symbol_  = false;
  enable_log_analyze_ = false;
  enable_log_ggs_     = true;

  enable_profiler_ = false;
  profiler_start_  = 0x00000000;
  profiler_end_    = 0x00000000;

  command_id_ = -1;

  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_WINDOW_ACTUAL_SIZE_SCREEN, actual_size_screen_ ? MF_CHECKED : MF_UNCHECKED);

  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_LOG_CPU, enable_log_cpu_ ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_LOG_GS, enable_log_gs_ ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_LOG_MEMORY, enable_log_mem_ ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_LOG_ELF, enable_log_elf_ ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_LOG_DMA, enable_log_dma_ ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_LOG_RECOMPILER, enable_log_rec_ ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_LOG_APP, enable_log_app_ ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_LOG_DDB, enable_log_ddb_ ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_LOG_SYMBOL, enable_log_symbol_ ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_LOG_ANALYZE, enable_log_analyze_ ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_LOG_GGS, enable_log_ggs_ ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(GetMenu(g_scrn->wnd_->getHwnd()), ID_PROFILER_ENABLE, enable_profiler_ ? MF_CHECKED : MF_UNCHECKED);

  // 等幅フォント
  LOGFONT  lf;
  memset(&lf, 0, sizeof(LOGFONT));
  lf.lfHeight      = 12;
  lf.lfWidth      = 0;
  lf.lfEscapement    = 0;
  lf.lfOrientation  = 0;
  lf.lfWeight      = FW_NORMAL;
  lf.lfItalic      = FALSE;
  lf.lfUnderline    = FALSE;
  lf.lfStrikeOut    = FALSE;
  lf.lfCharSet    = SHIFTJIS_CHARSET;
  lf.lfOutPrecision  = OUT_DEFAULT_PRECIS;
  lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
  lf.lfQuality    = DEFAULT_QUALITY;
  lf.lfPitchAndFamily  = DEFAULT_PITCH | FF_DONTCARE;
  strcpy(lf.lfFaceName, "ＭＳ ゴシック");
  fixed_font_ = CreateFontIndirect(&lf);
}

Debugger::~Debugger() {
  clear();

  delete register_window_;
  delete code_window_;
  delete memory_window_;
  delete breakpoint_window_;
  delete trace_window_;
  delete reference_window_;
  delete patch_window_;
  delete profiler_window_;
}

void Debugger::init(char* p_file) {

  // デフォルトのモジュールを登録
  module_ary_.push_back(new ModuleInfo("0x00000000[0x02000000]", 0x00000000, 0x02000000, 0x00000000, true));
  module_ary_.push_back(new ModuleInfo("0x20000000[0x02000000]", 0x20000000, 0x02000000, 0x20000000, true));
  module_ary_.push_back(new ModuleInfo("0x30000000[0x02000000]", 0x30000000, 0x02000000, 0x30000000, true));
  //module_ary_.add(new ModuleInfo("0x1fc00000[0x00400000]", 0x1fc00000, 0x00400000, 0x1fc00000, true));
  //module_ary_.add(new ModuleInfo("0x9fc00000[0x00400000]", 0x9fc00000, 0x00400000, 0x9fc00000, true));
  //module_ary_.add(new ModuleInfo("0xbfc00000[0x00400000]", 0xbfc00000, 0x00400000, 0xbfc00000, true));

  // 各種ウインドウの初期化
  register_window_->Init();
  code_window_->Init();
  memory_window_->Init();
  breakpoint_window_->Init();
  trace_window_->Init();
  reference_window_->Init();
  patch_window_->Init();
  profiler_window_->Init();

  // idx0にはアセンブラの情報を保持する
  src_file_info_ary_.push_back(new SourceFileInfo(NULL, 0, NULL));

  // ソースIDを置き換え
  for (int i = 0; i < static_cast<int>(src_loc_ary_line_.size()); i++) {
    char  *path = src_file_ary_[src_loc_ary_line_[i]->src_id]->path;
    int    sfi_idx = -1;

    // 既に追加済のソースか？
    for (int j = 1; j < static_cast<int>(src_file_info_ary_.size()); j++) {
      if (strcmp(path, src_file_info_ary_[j]->src_file()->path) == 0) {
        sfi_idx = j;
        break;
      }
    }
    // なければdwarfのソース一覧から探す
    if (sfi_idx == -1) {
      for (int j = 0; j < static_cast<int>(src_file_ary_.size()); j++) {
        if (strcmp(path, src_file_ary_[j]->path) == 0) {
          // 実際にファイルがあればリストに登録する
          FILE* fp = fopen(path, "rb");
          if (fp) {
            int fsize = ds_util::fsize32(fp);
            char* buf = (char*)malloc(fsize + 1);
            fread(buf, 1, fsize, fp);
            buf[fsize] = '\0';
            fclose(fp);
            src_file_info_ary_.push_back(new SourceFileInfo(buf, fsize, src_file_ary_[j]));
            sfi_idx = src_file_info_ary_.size() - 1;
            DBGOUT_SYM("new src file info [%02d]:path=%s\n", j, path);
          } else {
            DBGOUT_SYM("file \"%s\" not found.\n", path);
          }
          break;
        }
      }
    }
    // src_info_idにsrc_file_info_ary_のインデックスを設定
    if (sfi_idx >= 0) {
      DBGOUT_SYM("src_loc_ary_line_[%d]->src_info_id = %d\n", i, sfi_idx - 1);
      src_loc_ary_line_[i]->src_info_id = sfi_idx - 1;
    } else {
      src_loc_ary_line_[i]->src_info_id = -1;
    }
  }
  AddrJumpDialog::reset();

  clearDebugDatabase();
  cur_ddb_idx_ = -1;

  code_window_->ModuleSetup();

  reference_window_->UpdateBottleNeckEntries();

  clearMacroRecInfo();

  drawAllWindow();
}

void Debugger::clear() {
  reference_window_->OnClear();

  ModuleInfo* module = findELFModule();
  clearModuleInfo();

  clearDebugDatabase();

  cur_ddb_idx_ = -1;

  clearSymbol();

  clearCallStack();
  clearJumpLogEntry();

  clearString();
  analyze_map_.clear();

  clearFindItem();

  clearBottleNeck();
  clearMacroRecInfo();

  clearSource();
}

void Debugger::clearDebugDatabase() {
  for (DdbAryItr itr = ddb_ary_.begin();
      itr != ddb_ary_.end();
      ++itr) {
    delete *itr;
  }
  ddb_ary_.clear();
}

void Debugger::clearModuleInfo() {
  for (ModuleInfoAryItr itr = module_ary_.begin();
      itr != module_ary_.end();
      ++itr) {
    delete *itr;
  }
  module_ary_.clear();
}

void Debugger::clearFindItem() {
  for (FindItemAryItr itr = finditem_ary_.begin();
      itr != finditem_ary_.end();
      ++itr) {
    delete *itr;
  }
  finditem_ary_.clear();
}

void Debugger::clearBottleNeck() {
  for (BottleNeckAryItr itr = bottle_neck_ary_.begin();
      itr != bottle_neck_ary_.end();
      ++itr) {
    delete *itr;
  }
  bottle_neck_ary_.clear();
}

void Debugger::clearMacroRecInfo() {
  for (MacroRecInfoAryItr itr = macro_rec_ary_.begin();
      itr != macro_rec_ary_.end();
      ++itr) {
    delete *itr;
  }
  macro_rec_ary_.clear();
}

void Debugger::clearSymbol() {
  for (SymbolMapItr itr = symbol_map_.begin();
      itr != symbol_map_.end();
      ++itr) {
    delete itr->second;
  }
  symbol_map_.clear();
}

void Debugger::clearCallStack() {
  for (CallStackEntryAryItr itr = callstack_stk_.begin();
      itr != callstack_stk_.end();
      ++itr) {
    delete *itr;
  }
  callstack_stk_.clear();
}

void Debugger::clearJumpLogEntry() {
  for (JumpLogEntryAryItr itr = jumplog_ary_.begin();
      itr != jumplog_ary_.end();
      ++itr) {
    delete *itr;
  }
  jumplog_ary_.clear();
}

void Debugger::clearString() {
  for (StringMapItr itr = string_map_.begin();
      itr != string_map_.end();
      ++itr) {
    delete itr->second;
  }
  string_map_.clear();
}

void Debugger::clearSource() {
  for (SrcFileEntryAryItr itr = src_file_ary_.begin();
      itr != src_file_ary_.end();
      ++itr) {
    delete *itr;
  }
  src_file_ary_.clear();
  for (SrcLocEntryAryItr itr = src_loc_ary_addr_.begin();
      itr != src_loc_ary_addr_.end();
      ++itr) {
    delete *itr;
  }
  src_loc_ary_addr_.clear();
  src_loc_ary_line_.clear();
  for (SrcFileInfoAryItr itr = src_file_info_ary_.begin();
      itr != src_file_info_ary_.end();
      ++itr) {
    delete *itr;
  }
  src_file_info_ary_.clear();
}

bool Debugger::preTranslateMessage(MSG* p_msg) {
  if (code_window_->TranslateAccelerator(p_msg)) return true;

  char arrowkeys[64];
  sprintf(arrowkeys, "%c%c%c%c", VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN);

  if (register_window_->dlg()->dispatchDialogMessage(p_msg, NULL, "") == true) return true;
  
  if (code_window_->dlg()->dispatchDialogMessage(p_msg, GetDlgItem(code_window_->dlg()->getHwnd(), IDC_DUMMY), arrowkeys) == true) return true;

  if (memory_window_->dlg()->dispatchDialogMessage(p_msg, GetDlgItem(memory_window_->dlg()->getHwnd(), IDC_DUMMY), arrowkeys) == true) return true;

  if (breakpoint_window_->dlg()->dispatchDialogMessage(p_msg, NULL, "") == true) return true;

  if (trace_window_->dlg()->dispatchDialogMessage(p_msg, GetDlgItem(trace_window_->dlg()->getHwnd(), IDC_DUMMY), arrowkeys) == true) return true;

  if (reference_window_->dlg()->dispatchDialogMessage(p_msg, GetDlgItem(reference_window_->dlg()->getHwnd(), IDC_DUMMY), arrowkeys) == true) return true;

  if (patch_window_->dlg()->dispatchDialogMessage(p_msg, NULL, "") == true) return true;

  if (profiler_window_->dlg()->dispatchDialogMessage(p_msg, NULL, "") == true) return true;

  return false;
}

void Debugger::preExecute() {
  register_window_->OnPreExecute();
}

void Debugger::postExecute() {
  memory_window_->ShowCurrentAddress();
}

void* Debugger::createExecuteFiber() {
  return CreateFiber(0, executeProc, &command_id_);
}

void Debugger::loadAllDdb() {
  s8 dir_path[1024];
  sprintf(dir_path, "%s/data/%s", g_base_dir, g_elf_name);

  char find_pattern[1024];
  sprintf(find_pattern, "%s/*", dir_path);

  WIN32_FIND_DATA fd;
  HANDLE hdl = FindFirstFile(find_pattern, &fd);
  if (hdl != INVALID_HANDLE_VALUE) {
    do {
      if (strcmp(fd.cFileName, ".")  == 0 ||
          strcmp(fd.cFileName, "..") == 0) {
        continue;
      }

      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        // 下位ディレクトリまでは探索しない
      } else {
        if (strlen(fd.cFileName) >= 4 &&
            strcmp(&fd.cFileName[strlen(fd.cFileName) - 4], ".ddb") == 0) {
          char str[1024];
          sprintf(str, "%s/%s", dir_path, fd.cFileName);
          loadDdb(str);
        }
      }
    } while (FindNextFile(hdl, &fd));
  }
  // ファイルに設定されていた優先度でソート
  orderByPriority();

  breakpoint_window_->update_list();
  reference_window_->update_list();
  patch_window_->update_list();

  drawAllWindow();
}

void Debugger::loadDdb(const s8* p_fpath) {
  DebugDatabase* ddb = new DebugDatabase();
  ddb->load(p_fpath);
  ddb_ary_.push_back(DdbAryValue(ddb));
}

void Debugger::saveDdb() {
  for (DdbAryItr itr = ddb_ary_.begin();
      itr != ddb_ary_.end();
      ++itr) {
    (*itr)->save();
  }
}

void Debugger::newDdb(s8* p_ddb_name) {
  char str[1024];
  sprintf(str, "%s/data/%s/%s.ddb", g_base_dir, g_elf_name, p_ddb_name);

  DebugDatabase* ddb = new DebugDatabase();
  ddb->create(str);
  // 新規作成したら優先度最大でカレントDDBに設定する
  u32 max_prio = 0;
  for (u32 i = 0; i < g_dbg->ddb_ary_.size(); i++) {
    if (max_prio < g_dbg->ddb_ary_[i]->getPriority()) {
      max_prio = g_dbg->ddb_ary_[i]->getPriority();
    }
  }
  ddb->setPriority(max_prio + 1);
  ddb_ary_.push_back(DdbAryValue(ddb));
  cur_ddb_idx_ = ddb_ary_.size() - 1;
  MessageBox(g_scrn->hwnd(), "新しいDDBをカレントに設定します", NULL, NULL);
}

void Debugger::selectDdb(int p_idx) {
  if (p_idx >= 0 && p_idx < (int)ddb_ary_.size()) {
    cur_ddb_idx_ = p_idx;
  }
}

void Debugger::orderByPriority() {
  // 優先度に従ってDDB配列を並び替え
  DebugDatabase* cur = ddb_ary_[cur_ddb_idx_];
  struct local {
    static bool sort_func(const void* p_a, const void* p_b) {
      const DebugDatabase* a = static_cast<const DebugDatabase*>(p_a);
      const DebugDatabase* b = static_cast<const DebugDatabase*>(p_b);
      return a->getPriority() < b->getPriority();
    }
  };
  std::sort(ddb_ary_.begin(), ddb_ary_.end(), local::sort_func);
  u32 prio = 0;
  for (u32 i = 0; i < ddb_ary_.size(); i++) {
    ddb_ary_[i]->setPriority(prio++);
    if (ddb_ary_[i] == cur) {
      cur_ddb_idx_ = i;
    }
  }
}

s32 Debugger::getDDBIndex(const DebugDatabase* p_ddb_obj) {
  for (u32 i = 0; i < ddb_ary_.size(); i++) {
    if (ddb_ary_[i] == p_ddb_obj) return i;
  }
  return -1;
}

ModuleInfo* Debugger::findELFModule() {
  for (ModuleInfoAryItr itr = g_dbg->module_ary_.begin();
      itr != g_dbg->module_ary_.end();
      ++itr) {
    if ((*itr)->dummy() == false) {
      return *itr;
    }
  }
  return NULL;
}

bool Debugger::isInModuleCodeSection(u32 p_va) {
  for (ModuleInfoAryItr itr = g_dbg->module_ary_.begin();
      itr != g_dbg->module_ary_.end();
      ++itr) {
    if ((*itr)->dummy() == false &&
        p_va >= (*itr)->addr() &&
        p_va < (*itr)->addr() + (*itr)->size()) {
      return true;
    }
  }
  return false;
}

s8* Debugger::findSymbolTableName(u32 p_va) {
  SymbolMapItr itr = symbol_map_.find(p_va);
  return itr != symbol_map_.end() ? itr->second->name : NULL;
}

void Debugger::doStaticCodeAnalyze() {
  ModuleInfo* module = g_dbg->findELFModule();
  assert(module);

  for (SymbolMapItr itr = symbol_map_.begin();
      itr != symbol_map_.end();
      ++itr) {
    delete itr->second;
  }
  symbol_map_.clear();

  analyze_map_.clear();

  AnalyzeContext analyzer;

  analyzer.Init(module->entrypoint());
  analyzer.AnalyzeFragment(analyze_map_, true);

  for (u32 i = 0; i < module->size(); i += 4) {
    u32 va = module->addr() + i;
    if (analyze_map_.find(va) != analyze_map_.end()) continue;

    analyzer.Init(va);
    analyzer.AnalyzeFragment(analyze_map_, false);
  }
}

u32 Debugger::getMacroRecompileId(u32 p_va) {
  u32 macro_rec_id = -1;
  for (u32 i = 0; i < g_dbg->macro_rec_ary_.size(); i++) {
    if (g_dbg->macro_rec_ary_[i]->va() == p_va) {
      macro_rec_id = g_dbg->macro_rec_ary_[i]->pattern_id();
      break;
    }
  }
  return macro_rec_id;
}

void Debugger::drawAllWindow() {
  register_window_->gpr_update();
  register_window_->cop0_update();
  register_window_->fpr_update();
  code_window_->Draw();
  memory_window_->Draw();
  breakpoint_window_->Draw();
  trace_window_->Draw();
  reference_window_->Draw();
  patch_window_->Draw();
  profiler_window_->Draw();
}

void Debugger::foreAllWindow() {
  code_window_->dlg()->fore();
  memory_window_->dlg()->fore();
  breakpoint_window_->dlg()->fore();
  trace_window_->dlg()->fore();
  reference_window_->dlg()->fore();
  register_window_->dlg()->fore();
  patch_window_->dlg()->fore();
  profiler_window_->dlg()->fore();
}

HWND Debugger::findModalDialog() {
  s8 dlgnames[][64] = {
    "BreakPoint Info Dialog",
    "データを検索",
    "命令を検索",
    "範囲を指定してダンプ",
    "ジャンプ先を入力して移動",
    "Memory Edit Dialog",
    "Line Edit Dialog",
    "Patch Edit Dialog",
    NULL,
  };

  for (int i = 0; dlgnames[i][0] != NULL; i++) {
    HWND hwnd = FindWindow(MAKEINTRESOURCE(32770), dlgnames[i]);
    if (hwnd) return hwnd;
  }
  return NULL;
}

void Debugger::setFontToFixed(HWND p_hwnd) {
  SendMessage(p_hwnd, WM_SETFONT, (WPARAM)fixed_font_, (LPARAM)true);
}

void Debugger::setFontToFixed(HDC p_hdc) {
  SelectObject(p_hdc, fixed_font_);
}

void Debugger::allBreakOn() {
  for (int i = 0; i < g_dbg->get_breakpoint_count(); i++) {
    if (g_dbg->get_breakpoint(i)->enable()) {
      g_dbg->get_breakpoint(i)->breakOn();
    }
  }
}

void Debugger::allBreakOff() {
  for (int i = 0; i < g_dbg->get_breakpoint_count(); i++) {
    g_dbg->get_breakpoint(i)->breakOff();
  }
}

void Debugger::allPatchDisableWithSave() {
  // パッチの適用状態を保存しつつ無効にする
  // 新規パッチ作成前にこれを実行しオリジナルのデータに戻しておくこと！
  assert(save_patch_state_ary_.size() == 0);
  
  for (int i = 0; i < getPatchCount(); i++) {
    save_patch_state_ary_.push_back(deque<bool>::value_type(getPatch(i)->state()));
    getPatch(i)->SetDisable();
  }
}

void Debugger::patchRestore() {
  // allPatchDisableWithSave()で保存した状態を復元する
  // 新規パッチ作成後にこれを呼び出すこと！
  for (int i = 0; i < static_cast<int>(save_patch_state_ary_.size()); i++) {
    if (save_patch_state_ary_[i]) {
      getPatch(i)->SetEnable();
    }
  }
  save_patch_state_ary_.clear();
}

int Debugger::getJumpLogNestLevel(int p_idx, int p_ridx) {
  int level = 0;
  for (int i = 1; i < jumplog(p_idx)->m_repeat[p_ridx]->len; i++) {
    if (p_idx + i >= jumplog_count()) {
      break;
    }
  
    // 同一アドレスに複数のループブロックがある場合、
    // 配列の末尾に最もレベルが高いものが置かれる
    int ridx = jumplog(p_idx + i)->m_repeat.size() - 1;
    if (ridx >= 0) {
      int tmp = getJumpLogNestLevel(p_idx + i, ridx) + ridx;
      if (level < tmp) {
        level = tmp;
      }
    }
  }
  return level + 1;
}

void Debugger::jumpLogPush() {
  if (g_dbg->without_debugger_inspection()) return;

  if (g_dbg->jumplog_ary_.size() > TraceWindow::kJumpLog_Size) {
    delete g_dbg->jumplog_ary_.front();
    g_dbg->jumplog_ary_.erase(&g_dbg->jumplog_ary_.front());
  }

  g_dbg->jumplog_ary_.push_back(new JumpLogEntry(g_cpu->m_pc - 4));

  for (int i = 1; i < JumpLogEntry::kGroupMaxLen; i++) {
    if (static_cast<int>(g_dbg->jumplog_ary_.size()) < i * 2) {
      break;
    }
    int new_tail_idx = g_dbg->jumplog_ary_.size() - 1;
    int old_tail_idx = g_dbg->jumplog_ary_.size() - 1 - i;
    int old_head_idx = g_dbg->jumplog_ary_.size() - i * 2;
    if (isMatchJumpLog(new_tail_idx, old_tail_idx, i)) {
      for (int j = 0; j < i; j++) {
        delete g_dbg->jumplog_ary_.back();
        g_dbg->jumplog_ary_.erase(&g_dbg->jumplog_ary_.back());
      }
      int rep_cnt = g_dbg->jumplog_ary_[old_head_idx]->m_repeat.size();
      if (rep_cnt > 0 &&
          g_dbg->jumplog_ary_[old_head_idx]->
            m_repeat[rep_cnt - 1]->len == i) {
        // 既にループが存在しているのでカウントのみをインクリメント
        g_dbg->jumplog_ary_[old_head_idx]->m_repeat[rep_cnt - 1]->count++;
      } else {
        // 存在しないループは新規作成
        JumpLogRepeat* newrep = new JumpLogRepeat;
        newrep->len = i;
        newrep->count = 2;
        g_dbg->jumplog_ary_[old_head_idx]->m_repeat.push_back(newrep);
      }
    }
  }
}

void Debugger::callStackPush(u32 p_jva) {
  g_dbg->callstack_stk_.push_back(new CallStackEntry(g_cpu->m_pc - 4, p_jva));
}

void Debugger::callStackPop(u32 p_jva) {
  if (g_dbg->callstack_stk_.size() <= 0) {
    DBGOUT_CPU("リターン命令が呼ばれましたが、"
               "対応するサブルーチンコールがありません！\n");
    return;
  }

  int find_depth = 0;
  for (int i = g_dbg->callstack_stk_.size() - 1; i >= 0; i--) {
    if (g_dbg->callstack_stk_[i]->va() + 8 == p_jva) {
      for (int j = 0; j <= find_depth; j++) {
        delete g_dbg->callstack_stk_.back();
        g_dbg->callstack_stk_.pop_back();
      }
      break;
    } else {
      find_depth++;
    }
  }
  if (find_depth > 0) {
    u32 opcode = *((u32*)VA2PA(g_cpu->m_pc));
    if (opcode && ((opcode >> 21) & 0x1f) != 0x1f) {
      //DBGOUT_CPU("リターンアドレスは最後の呼び出し元ではありません！"
      //           "コールスタックは正しくないかもしれません\n");
    }
  } else if (find_depth == g_dbg->callstack_stk_.size()) {
    DBGOUT_CPU("リターンアドレスがコールスタックから見つかりません！\n");
  }
}

void Debugger::setupParserVars() {
  variable_delete_all();
  variable_add("pc", (u32)&g_cpu->m_pc, Expr_Val::kUint, 4, 1, true);
  variable_add("sa", (u32)&g_cpu->m_sa, Expr_Val::kUint, 4, 1, true);
  variable_add("hi", (u32)&g_cpu->m_hi, Expr_Val::kUint, 4, 4, true);
  variable_add("lo", (u32)&g_cpu->m_lo, Expr_Val::kUint, 4, 4, true);

  variable_add("zero", (u32)&g_cpu->m_ze.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("at", (u32)&g_cpu->m_at.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("v0", (u32)&g_cpu->m_v0.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("v1", (u32)&g_cpu->m_v1.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("a0", (u32)&g_cpu->m_a0.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("a1", (u32)&g_cpu->m_a1.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("a2", (u32)&g_cpu->m_a2.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("a3", (u32)&g_cpu->m_a3.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("t0", (u32)&g_cpu->m_t0.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("t1", (u32)&g_cpu->m_t1.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("t2", (u32)&g_cpu->m_t2.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("t3", (u32)&g_cpu->m_t3.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("t4", (u32)&g_cpu->m_t4.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("t5", (u32)&g_cpu->m_t5.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("t6", (u32)&g_cpu->m_t6.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("t7", (u32)&g_cpu->m_t7.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("s0", (u32)&g_cpu->m_s0.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("s1", (u32)&g_cpu->m_s1.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("s2", (u32)&g_cpu->m_s2.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("s3", (u32)&g_cpu->m_s3.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("s4", (u32)&g_cpu->m_s4.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("s5", (u32)&g_cpu->m_s5.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("s6", (u32)&g_cpu->m_s6.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("s7", (u32)&g_cpu->m_s7.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("t8", (u32)&g_cpu->m_t8.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("t9", (u32)&g_cpu->m_t9.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("k0", (u32)&g_cpu->m_k0.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("k1", (u32)&g_cpu->m_k1.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("gp", (u32)&g_cpu->m_gp.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("sp", (u32)&g_cpu->m_sp.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("fp", (u32)&g_cpu->m_fp.d[0], Expr_Val::kUint, 4, 4, true);
  variable_add("ra", (u32)&g_cpu->m_ra.d[0], Expr_Val::kUint, 4, 4, true);

  for (int i = 0; i < 32; i++) {
    s8 str[16];
    sprintf(str, "fpr%d", i);
    variable_add(str, (u32)&g_cpu->m_fpu.fpr[i], Expr_Val::kFloat, 4, 1, true);
  }

  for (int i = 0; i < getMemLabelCount(); i++) {
    MemLabel* mem_label = getMemLabel(i);

    int TYPE_CONV[] = {
      Expr_Val::kUint,  // hex
      Expr_Val::kInt,   // int8
      Expr_Val::kUint,  // uint8
      Expr_Val::kInt,   // int16
      Expr_Val::kUint,  // uint16
      Expr_Val::kInt,   // int32
      Expr_Val::kUint,  // uint32
      Expr_Val::kInt,   // int64
      Expr_Val::kUint,  // uint64
      Expr_Val::kFloat, // float
      Expr_Val::kFloat, // double
      Expr_Val::kUint,  // string
      Expr_Val::kUint,  // pointer
    };

    // メモリラベル名については制限しないが、
    // 変数名の条件を満たしていないと式で使用できない。

    int size = 4;
    int type = Expr_Val::kUint;
    
    if (mem_label->element_size() == 1) {
      // メンバが１つだけなら要素の型をそのまま利用する
      int type = TYPE_CONV[mem_label->element(0)->type];
      int size = mem_label->element(0)->size;
      variable_add(
        mem_label->text(), (u32)mem_label->va(), type,
        size, mem_label->count(), false);
    } else {
      variable_add(
        mem_label->text(), (u32)mem_label->va(), Expr_Val::kError,
        mem_label->GetStructureSize(), mem_label->count(), false);

      int offset = 0;
      if (mem_label->element_size() > 1) {
        for (int j = 0; j < mem_label->element_size(); j++) {
          variable_add_member(
            mem_label->text(),
            mem_label->element(j)->label,
            offset,
            TYPE_CONV[mem_label->element(j)->type],
            mem_label->element(j)->size);
          offset += mem_label->element(j)->size;
        }
      }
    }
  }
}

void Debugger::cleanupParserVars() {
  variable_delete_all();
}

bool Debugger::isMatchJumpLog(int p_idx1, int p_idx2, int p_len) {
  for (int i = 0; i < p_len; i++) {
    JumpLogEntry* log1 = g_dbg->jumplog_ary_[p_idx1 - i];
    JumpLogEntry* log2 = g_dbg->jumplog_ary_[p_idx2 - i];

    if (log1->va() != log2->va()) {
      return false;
    }

    // log1のリピートオブジェクトがlog2と一致するかチェック
    // 但し、log1のリピートオブジェクト数がlog2よりも少ないときは
    // log2の最外部のリピートオブジェクトは一致する必要はない
    for (u32 j = 0; j < log1->m_repeat.size(); j++) {
      if (j >= log2->m_repeat.size()) {
        // log2の方が少ないことはありえないので不一致とみなす
        return false;
      }
      if (log1->m_repeat[j]->count != log2->m_repeat[j]->count ||
          log1->m_repeat[j]->len != log2->m_repeat[j]->len) {
        return false;
      }
    }
  }
  return true;
}

void CALLBACK Debugger::executeProc(void* lpParameter) {
  int* executeType = (int*)lpParameter;
  while (true) {
    switch (*executeType) {
    case ID_DEBUG_RUN_WITHOUT_BREAK:
      g_dbg->set_without_debugger_inspection(true);
      g_dbg->preExecute();

      g_cpu->execute();
      
      g_dbg->postExecute();

      // pcの位置+３行に自動スクロール
      g_dbg->code_window_->AutoJumpModule(g_cpu->m_pc + 12);
      g_dbg->code_window_->ScrollTo(
        g_dbg->code_window_->VA2Line(g_cpu->m_pc));
      g_dbg->drawAllWindow();
      break;
    case ID_DEBUG_RUN: {
      g_dbg->set_without_debugger_inspection(false);
      g_dbg->preExecute();

      BreakPointBase* bkpt = NULL;
      while (1) {
        // PCにブレークポイントがあるとそこから進まなくなるので
        // PCにある命令を一度ステップ実行してからRUNする
        if (g_cpu->m_ee_jump_addr == 0xffffffff) {
          g_dbg->step_breakpoint_.set_va(g_cpu->m_pc + 4);
        } else {
          g_dbg->step_breakpoint_.set_va(g_cpu->m_ee_jump_addr);
        }

        if (bkpt == NULL) {
          // 初回
          g_dbg->step_breakpoint_.breakOn();
          g_cpu->execute();
          g_dbg->step_breakpoint_.breakOff();
        } else {
          // 一度ブレークして条件不一致で再実行する場合
          // ステップブレークはスレッド切り替え時など
          // 実行中に張り替えることがありえるので
          // ユーザブレークよりも後で設定する必要がある。
          bkpt->breakOff();
          g_dbg->step_breakpoint_.breakOn();
          g_cpu->execute();
          g_dbg->step_breakpoint_.breakOff();
          bkpt->breakOn();
        }

        g_dbg->allBreakOn();
        g_cpu->execute();
        g_dbg->allBreakOff();

        int idx;
        u32 va = g_dbg->hit_mem_break_ ? g_dbg->hit_mem_break_va_ : g_cpu->m_pc;
        int ofs = g_dbg->get_breakpoint_idx(va, &idx, false);
        if (ofs == -1) {
          // 止まっているのにブレークポイントがない場合、
          // 不正なメモリアクセスと見なす
          break;
        } else {
          bkpt = g_dbg->get_breakpoint(idx);
          if (bkpt->evaluateExpression() != 0) {
            // 条件一致した場合、停止
            break;
          }
          // 条件一致しない場合、そのまま継続して実行する
        }
      }
      
      g_dbg->postExecute();

      // pcの位置+３行に自動スクロール
      g_dbg->code_window_->AutoJumpModule(g_cpu->m_pc + 12);
      g_dbg->code_window_->ScrollTo(
        g_dbg->code_window_->VA2Line(g_cpu->m_pc));
      g_dbg->drawAllWindow();
      break; }
    case ID_DEBUG_STEPIN:
      g_dbg->set_without_debugger_inspection(false);
      g_dbg->preExecute();
  
      if (g_cpu->m_ee_jump_addr == 0xffffffff) {
        g_dbg->step_breakpoint_.set_va(g_cpu->m_pc + 4);
      } else {
        g_dbg->step_breakpoint_.set_va(g_cpu->m_ee_jump_addr);
      }
      g_dbg->step_breakpoint_.breakOn();
      g_cpu->execute();
      g_dbg->step_breakpoint_.breakOff();

      g_dbg->postExecute();

      // pcの位置+３行に自動スクロール
      g_dbg->code_window_->AutoJumpModule(g_cpu->m_pc + 12);
      g_dbg->code_window_->ScrollTo(
        g_dbg->code_window_->VA2Line(g_cpu->m_pc));
      g_dbg->drawAllWindow();
      break;
    case ID_DEBUG_STEPOVER:
      g_dbg->set_without_debugger_inspection(false);
      g_dbg->preExecute();

      u32 opcode = *((u32*)VA2PA(g_cpu->m_pc));
      
      // jal or jalr
      bool subroutine = (opcode >> 26 == 3 ||
                         opcode >> 26 == 0 && opcode >> 26 == 9);

      u32  func_ret = 0xffffffff;
      BreakPointBase* bkpt = NULL;
      while (1) {
        // PCにブレークポイントがあるとそこから進まなくなるので
        // PCにある命令を一度ステップ実行してからRUNする
        if (g_cpu->m_ee_jump_addr == 0xffffffff) {
          g_dbg->step_breakpoint_.set_va(g_cpu->m_pc + 4);
        } else {
          g_dbg->step_breakpoint_.set_va(g_cpu->m_ee_jump_addr);
        }

        if (bkpt == NULL) {
          // 初回
          g_dbg->step_breakpoint_.breakOn();
          g_cpu->execute();
          g_dbg->step_breakpoint_.breakOff();
          // 関数の出口を保存
          func_ret = g_cpu->m_pc + 4;
        } else {
          // 一度ブレークして条件不一致で再実行する場合
          // ステップブレークはスレッド切り替え時など
          // 実行中に張り替えることがありえるので
          // ユーザブレークよりも後で設定する必要がある。
          bkpt->breakOff();
          g_dbg->step_breakpoint_.breakOn();
          g_cpu->execute();
          g_dbg->step_breakpoint_.breakOff();
          bkpt->breakOn();
        }

        if (subroutine == false) break;

        // ステップブレークを関数の出口にセットし実行
        // ステップブレークはスレッド切り替え時など
        // 実行中に張り替えることがありえるので
        // ユーザブレークよりも後で設定する必要がある。
        g_dbg->allBreakOn();
        g_dbg->step_breakpoint_.set_va(func_ret);
        g_dbg->step_breakpoint_.breakOn();
        g_cpu->execute();
        g_dbg->step_breakpoint_.breakOff();
        g_dbg->allBreakOff();

        int idx;
        u32 va = g_dbg->hit_mem_break_ ? g_dbg->hit_mem_break_va_ : g_cpu->m_pc;
        int ofs = g_dbg->get_breakpoint_idx(va, &idx, false);
        if (ofs == -1) {
          // 止まっているのにブレークポイントがない場合、
          // 不正なメモリアクセスと見なす
          break;
        } else {
          bkpt = g_dbg->get_breakpoint(idx);
          if (bkpt->evaluateExpression() != 0) {
            // 条件一致した場合、停止
            break;
          }
          // 条件一致しない場合、そのまま継続して実行する
        }
      }

      g_dbg->postExecute();

      // pcの位置+３行に自動スクロール
      g_dbg->code_window_->AutoJumpModule(g_cpu->m_pc + 12);
      g_dbg->code_window_->ScrollTo(
        g_dbg->code_window_->VA2Line(g_cpu->m_pc));
      g_dbg->drawAllWindow();
      break;
    }
    g_dbg->set_command_id(-1);
    SwitchToFiber(g_app.main_fiber());
  }
}

ModuleInfo* Debugger::module(int p_idx) {
  return module_ary_[p_idx];
}

int Debugger::module_count() {
  return module_ary_.size();
}

void Debugger::module_insert(int p_idx, class ModuleInfo* p_obj) {
  module_ary_.insert(&module_ary_[p_idx], p_obj);
}

// debug database access
s8* Debugger::get_current_ddb_name() {
  if (cur_ddb_idx_ >= 0 && cur_ddb_idx_ < (int)ddb_ary_.size()) {
    return ddb_ary_[cur_ddb_idx_]->file_name();
  } else {
    return NULL;
  }
}

DebugDatabase* Debugger::get_current_ddb() {
  if (g_dbg->cur_ddb_idx_ >= 0) {
    return g_dbg->ddb_ary_[g_dbg->cur_ddb_idx_];
  } else {
    return NULL;
  }
}

int Debugger::get_breakpoint_count() {
  int count = 0;
  for (DdbAryItr itr = ddb_ary_.begin();
      itr != ddb_ary_.end();
      ++itr) {
    count += (*itr)->getBreakPointCount();
  }
  return count;
}

BreakPointBase* Debugger::get_breakpoint(int p_idx) {
  int ary_idx, part_idx;
  if (getPartDdbIdx(p_idx, &ary_idx, &part_idx,
                    &DebugDatabase::getBreakPointCount)) {
    return ddb_ary_[ary_idx]->getBreakPoint(part_idx);
  } else {
    return NULL;
  }
}

int Debugger::get_breakpoint_idx(u32 p_va, int* p_idx, bool p_cur_only) {
  // p_vaを跨ぐブレークポイントが存在する場合、
  // アドレスの先頭からのオフセットを返す
  // 存在しない場合は-1
  for (int i = 0; i < (int)ddb_ary_.size(); i++) {
    int local_idx;
    int result = ddb_ary_[i]->getBreakPointIdx(p_va, &local_idx);
    if (result != -1) {
      if (p_cur_only && !ddb_ary_[i]->getBreakPoint(local_idx)->is_cur()) {
        continue;
      }
      if (p_idx) {
        getWholeDdbIdx(i, local_idx, p_idx,
                       &DebugDatabase::getBreakPointCount);
      }
      return result;
    }
  }
  if (p_idx) *p_idx = -1;
  return -1;
}

void Debugger::add_breakpoint(BreakPointBase* p_data) {
  ddb_ary_[cur_ddb_idx_]->addBreakPoint(p_data);
  breakpoint_window_->add_entry(p_data);
}

void Debugger::delete_breakpoint(int p_idx) {
  int ary_idx, part_idx;
  if (getPartDdbIdx(p_idx, &ary_idx, &part_idx,
        &DebugDatabase::getBreakPointCount)) {
    breakpoint_window_->remove_entry(
      ddb_ary_[ary_idx]->getBreakPoint(part_idx));
    ddb_ary_[ary_idx]->deleteBreakPoint(part_idx);
  }
}

int Debugger::get_comment_count() {
  int count = 0;
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    count += ddb_ary_[i]->getCommentCount();
  }
  return count;
}

Comment* Debugger::get_comment(int p_idx) {
  int ary_idx, local_idx;
  if (getPartDdbIdx(p_idx, &ary_idx, &local_idx,
                    &DebugDatabase::getCommentCount)) {
    return ddb_ary_[ary_idx]->getComment(local_idx);
  } else {
    return NULL;
  }
}

int Debugger::get_comment_idx(u32 p_va) {
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    int local_idx = ddb_ary_[i]->getCommentIdx(p_va);
    if (local_idx != -1) {
      int global_idx;
      getWholeDdbIdx(i, local_idx, &global_idx,
                     &DebugDatabase::getBreakPointCount);
      return global_idx;
    }
  }
  return -1;
}

void Debugger::add_comment(Comment* p_data) {
  ddb_ary_[cur_ddb_idx_]->addComment(p_data);
  reference_window_->add_entry(p_data);
}

void Debugger::delete_comment(int p_idx) {
  int array_idx, local_idx;
  if (getPartDdbIdx(p_idx, &array_idx, &local_idx,
                    &DebugDatabase::getCommentCount)) {
    reference_window_->remove_entry(
      ddb_ary_[array_idx]->getComment(local_idx));
    ddb_ary_[array_idx]->deleteComment(local_idx);
  }
}

Comment* Debugger::find_comment(u32 p_va) {
  for (u32 i = 0; i < ddb_ary_.size(); i++) {
    Comment* comment = ddb_ary_[i]->findComment(p_va);
    if (comment) return comment;
  }
  return NULL;
}

//   ----------------------------------------------------------
//   code label methods
int Debugger::get_codelabel_count() {
  int count = 0;
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    count += ddb_ary_[i]->getCodeLabelCount();
  }
  return count;
}

Label* Debugger::get_codelabel(int p_idx) {
  int array_idx, local_idx;
  if (getPartDdbIdx(p_idx, &array_idx, &local_idx,
                    &DebugDatabase::getCodeLabelCount)) {
    return ddb_ary_[array_idx]->getCodeLabel(local_idx);
  } else {
    return NULL;
  }
}

int Debugger::get_codelabel_idx(u32 p_va) {
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    int local_idx = ddb_ary_[i]->getCodeLabelIdx(p_va);
    if (local_idx != -1) {
      int global_idx;
      getWholeDdbIdx(i, local_idx, &global_idx,
                     &DebugDatabase::getCodeLabelCount);
      return global_idx;
    }
  }
  return -1;
}

void Debugger::add_codelabel(Label* p_data) {
  ddb_ary_[cur_ddb_idx_]->addCodeLabel(p_data);
  reference_window_->add_entry(p_data);
}

void Debugger::delete_codelabel(int p_idx) {
  int ary_idx, local_idx;
  if (getPartDdbIdx(p_idx, &ary_idx, &local_idx,
                    &DebugDatabase::getCodeLabelCount)) {
    reference_window_->remove_entry(
      ddb_ary_[ary_idx]->getCodeLabel(local_idx));
    ddb_ary_[ary_idx]->deleteCodeLabel(local_idx);
  }
}

Label* Debugger::find_codelabel(u32 p_va) {
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    Label* data = ddb_ary_[i]->findCodeLabel(p_va);
    if (data) return data;
  }
  return NULL;
}

int Debugger::getMemLabelCount() {
  int count = 0;
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    count += ddb_ary_[i]->getMemLabelCount();
  }
  return count;
}

MemLabel* Debugger::getMemLabel(int p_idx) {
  int array_idx, part_idx;
  if (getPartDdbIdx(p_idx, &array_idx, &part_idx,
                    &DebugDatabase::getMemLabelCount)) {
    return ddb_ary_[array_idx]->getMemLabel(part_idx);
  } else {
    return NULL;
  }
}

int Debugger::getMemLabelIdx(u32 p_va, int* p_idx) {
  // p_vaを跨ぐメモリラベルが存在する場合、
  // アドレスの先頭からのオフセットを返す
  // 存在しない場合は-1
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    int part_idx;
    int result = ddb_ary_[i]->getMemLabelIdx(p_va, &part_idx);
    if (result != -1) {
      if (p_idx) {
        getWholeDdbIdx(i, part_idx, p_idx,
                       &DebugDatabase::getMemLabelCount);
      }
      return result;
    }
  }
  if (p_idx) *p_idx = -1;
  return -1;
}

void Debugger::addMemLabel(MemLabel* p_data) {
  ddb_ary_[cur_ddb_idx_]->addMemLabel(p_data);
  reference_window_->add_entry(p_data);
}

void Debugger::deleteMemLabel(int p_idx) {
  int ary_idx, part_idx;
  if (getPartDdbIdx(p_idx, &ary_idx, &part_idx,
                    &DebugDatabase::getMemLabelCount)) {
    reference_window_->remove_entry(ddb_ary_[ary_idx]->getMemLabel(part_idx));
    ddb_ary_[ary_idx]->deleteMemLabel(part_idx);
  }
}

MemLabel* Debugger::findMemLabel(u32 p_va) {
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    MemLabel* data = ddb_ary_[i]->findMemLabel(p_va);
    if (data) return data;
  }
  return NULL;
}

int Debugger::getPatchCount() {
  int count = 0;
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    count += ddb_ary_[i]->getPatchCount();
  }
  return count;
}

PatchEntry* Debugger::getPatch(int p_idx) {
  int ary_idx, part_idx;
  if (getPartDdbIdx(p_idx, &ary_idx, &part_idx,
                    &DebugDatabase::getPatchCount)) {
    return ddb_ary_[ary_idx]->getPatch(part_idx);
  } else {
    return NULL;
  }
}

void Debugger::getPatchIdx(PatchEntry* p_entry, int* p_idx) {
  // 全体インデックスを返す
  int whole_index = 0;
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    for (int j = 0; j < ddb_ary_[i]->getPatchCount(); j++) {
      if (ddb_ary_[i]->getPatch(j) == p_entry) {
        *p_idx = whole_index;
        return;
      }
      whole_index++;
    }
  }
  *p_idx = NULL;
  return;
}

void Debugger::addPatch(PatchEntry* p_data) {
  ddb_ary_[cur_ddb_idx_]->addPatch(p_data);
  patch_window_->add_entry(p_data);
}

void Debugger::deletePatch(int p_idx) {
  int ary_idx, part_idx;
  if (getPartDdbIdx(p_idx, &ary_idx, &part_idx,
                    &DebugDatabase::getPatchCount)) {
    patch_window_->remove_entry(ddb_ary_[ary_idx]->getPatch(part_idx));
    ddb_ary_[ary_idx]->deletePatch(part_idx);
  }
}

PatchEntry* Debugger::findPatch(u32 p_va,
                                u32 p_size,
                                bool p_ignore_disable) {
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    PatchEntry* data =
      ddb_ary_[i]->findPatch(p_va, p_size, p_ignore_disable);
    if (data) return data;
  }
  return NULL;
}

bool Debugger::unionPatch(PatchEntry* p_entry) {
  int idx;
  getPatchIdx(p_entry, &idx);
  for (int i = 0; i < getPatchCount(); i++) {
    if (i == idx) continue;
    
    PatchEntry* pat1 = getPatch(i);
    PatchEntry* pat2 = getPatch(idx);

    if (pat1->va() + pat1->size() == pat2->va()) {
      // 結合可能なパッチが存在する
      bool state = pat1->state();
      pat1->SetDisable();
      pat2->SetDisable();

      // 統合後のデータを作成
      int size = pat1->size() + pat2->size();
      s8* buf = new s8[size];
      memcpy(buf, pat1->new_data(), pat1->size());
      memcpy(buf + pat1->size(), pat2->new_data(), pat2->size());

      // 統合対象を削除
      deletePatch(idx);

      pat1->update(0, size, buf);
      delete[] buf;

      // 統合したパッチを元の状態に
      if (state) pat1->SetEnable();

      return true;
    }
  }
  return false;
}

int Debugger::getMemDumpLabelCount() {
  int count = 0;
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    count += ddb_ary_[i]->getMemDumpLabelCount();
  }
  return count;
}

MemDumpLabel* Debugger::getMemDumpLabel(int p_idx) {
  int ary_idx, part_idx;
  if (getPartDdbIdx(p_idx, &ary_idx, &part_idx,
                    &DebugDatabase::getMemDumpLabelCount)) {
    return ddb_ary_[ary_idx]->getMemDumpLabel(part_idx);
  } else {
    return NULL;
  }
}

void Debugger::addMemDumpLabel(MemDumpLabel* p_data) {
  ddb_ary_[cur_ddb_idx_]->addMemDumpLabel(p_data);
}

void Debugger::deleteMemDumpLabel(int p_idx) {
  int array_idx, part_idx;
  if (getPartDdbIdx(p_idx, &array_idx, &part_idx,
                       &DebugDatabase::getMemDumpLabelCount)) {
    ddb_ary_[array_idx]->deleteMemDumpLabel(part_idx);
  }
}

int Debugger::getProfileCount() {
  int count = 0;
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    count += ddb_ary_[i]->getProfileCount();
  }
  return count;
}

ProfileEntry* Debugger::getProfile(int p_idx) {
  int array_idx, part_idx;
  if (getPartDdbIdx(p_idx, &array_idx, &part_idx,
                    &DebugDatabase::getProfileCount)) {
    return ddb_ary_[array_idx]->getProfile(part_idx);
  } else {
    return NULL;
  }
}

bool Debugger::getPartDdbIdx(int p_whole_idx,
                             int* p_array_idx,
                             int* p_part_idx,
                             int(DebugDatabase::*p_count_proc)()) {
  // 全DDBの通しインデックスからDDB固有インデックスに変換する
  int tmpidx = p_whole_idx;
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    if (tmpidx < (ddb_ary_[i]->*p_count_proc)()) {
      *p_array_idx = i;
      *p_part_idx = tmpidx;
      return true;
    }
    tmpidx -= (ddb_ary_[i]->*p_count_proc)();
  }
  *p_array_idx = -1;
  *p_part_idx = -1;
  return false;
}

bool Debugger::getWholeDdbIdx(int p_array_idx,
                              int p_part_idx,
                              int* p_whole_idx,
                              int(DebugDatabase::*p_count_proc)()) {
  // DDB固有インデックスから全DDBの通しインデックスに変換する
  int tmpidx = p_part_idx;
  for (int i = 0; i < static_cast<int>(ddb_ary_.size()); i++) {
    if (i == p_array_idx) {
      *p_whole_idx = tmpidx;
      return true;
    }
    tmpidx +=  (ddb_ary_[i]->*p_count_proc)();
  }
  *p_whole_idx = -1;
  return false;
}
