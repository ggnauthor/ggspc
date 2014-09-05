#ifndef _debugger_debugger
#define _debugger_debugger

#pragma comment(lib, "comctl32.lib")

#include "main.h"
#include "stlp_wrapper.h"
#include "debugger/debug_database.h"
#include "debugger/static_code_analyze.h"
#include "debugger/module_info.h"
#include "debugger/macro_rec_info.h"
#include "debugger/jump_log_entry.h"
#include "debugger/call_stack_entry.h"

//#include <commctrl.h>

struct Symbol;
class  RegisterWindow;
class  CodeWindow;
class  MemoryWindow;
class  BreakPointWindow;
class  TraceWindow;
class  ReferenceWindow;
class  PatchWindow;
class  ProfilerWindow;
class  AnalyzeInfo;
struct SourceFileEntry;
struct SourceLocationEntry;
class  SourceFileInfo;

typedef hash_map<int, AnalyzeInfo>::iterator   AnalyzeInfoMapItr;
typedef hash_map<int, String*>::iterator       StringMapItr;
typedef hash_map<int, String*>::value_type     StringMapValue;
typedef hash_map<int, Symbol*>::iterator       SymbolMapItr;
typedef hash_map<int, Symbol*>::value_type     SymbolMapValue;
typedef vector<ModuleInfo*>::iterator          ModuleInfoAryItr;
typedef vector<ModuleInfo*>::value_type        ModuleInfoAryValue;
typedef vector<DebugDatabase*>::iterator       DdbAryItr;
typedef vector<DebugDatabase*>::value_type     DdbAryValue;
typedef vector<JumpLogEntry*>::iterator        JumpLogEntryAryItr;
typedef vector<JumpLogEntry*>::value_type      JumpLogEntryAryValue;
typedef vector<CallStackEntry*>::iterator      CallStackEntryAryItr;
typedef vector<CallStackEntry*>::value_type    CallStackEntryAryValue;
typedef vector<FindItem*>::iterator            FindItemAryItr;
typedef vector<FindItem*>::value_type          FindItemAryValue;
typedef vector<BottleNeck*>::iterator          BottleNeckAryItr;
typedef vector<BottleNeck*>::value_type        BottleNeckAryValue;
typedef vector<MacroRecInfo*>::iterator        MacroRecInfoAryItr;
typedef vector<MacroRecInfo*>::value_type      MacroRecInfoAryValue;

typedef vector<SourceFileEntry*>::iterator       SrcFileEntryAryItr;
typedef vector<SourceFileEntry*>::value_type     SrcFileEntryAryValue;
typedef vector<SourceLocationEntry*>::iterator   SrcLocEntryAryItr;
typedef vector<SourceLocationEntry*>::value_type SrcLocEntryAryValue;
typedef vector<SourceFileInfo*>::iterator        SrcFileInfoAryItr;
typedef vector<SourceFileInfo*>::value_type      SrcFileInfoAryValue;

/*-------*/
/* class */
/*-------*/
struct TypeDefinition {
  char type_str[8];
  int  size;    // 可変の場合は 0
  bool pointer;  // ポインタならtrue
};

class Debugger {
  friend LRESULT CALLBACK MemDumpDlgProc(
    HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
  friend LRESULT CALLBACK PatchInfoDlgProc(
    HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
  Debugger();
  ~Debugger();

  void init(char* p_file);
  void clear();

  void clearDebugDatabase();
  void clearModuleInfo();
  void clearFindItem();
  void clearCallStack();
  void clearJumpLogEntry();
  void clearBottleNeck();
  void clearMacroRecInfo();
  void clearSymbol();
  void clearString();
  void clearSource();

  bool  preTranslateMessage(MSG* p_msg);
  void  preExecute();
  void  postExecute();  
  void* createExecuteFiber();

  // Debug Database File
  void loadAllDdb();
  void loadDdb(const s8* p_fpath);
  void saveDdb();
  void newDdb(s8* p_filename);
  void selectDdb(int p_idx);
  void orderByPriority();
  s32  getDDBIndex(const DebugDatabase* p_ddb_obj);

  // Module
  ModuleInfo* findELFModule();
  bool isInModuleCodeSection(u32 p_va);
  s8*  findSymbolTableName(u32 p_va);

  // Analyze
  void doStaticCodeAnalyze();
  u32  getMacroRecompileId(u32 p_va);

  // Gui
  void drawAllWindow();
  void foreAllWindow();
  HWND findModalDialog();
  void setFontToFixed(HWND p_hwnd);
  void setFontToFixed(HDC p_hdc);

  // BreakPoint
  void allBreakOn();
  void allBreakOff();

  // Patch
  void allPatchDisableWithSave();
  void patchRestore();
  
  // JumpLog/CallStack
  int getJumpLogNestLevel(int p_idx, int p_ridx);
  static void jumpLogPush();
  static void callStackPush(u32 p_jva);
  static void callStackPop(u32 p_jva);

  // Parser
  void setupParserVars();
  void cleanupParserVars();

private:
  static bool isMatchJumpLog(int p_idx1, int p_idx2, int p_len);
  static void CALLBACK executeProc(void* lpParameter);

// accessor
public:
  inline bool actual_size_screen() { return actual_size_screen_; }
  inline bool enable_log_cpu() { return enable_log_cpu_; }
  inline bool enable_log_gs() { return enable_log_gs_; }
  inline bool enable_log_mem() { return enable_log_mem_; }
  inline bool enable_log_elf() { return enable_log_elf_; }
  inline bool enable_log_dma() { return enable_log_dma_; }
  inline bool enable_log_rec() { return enable_log_rec_; }
  inline bool enable_log_app() { return enable_log_app_; }
  inline bool enable_log_ddb() { return enable_log_ddb_; }
  inline bool enable_log_symbol() { return enable_log_symbol_; }
  inline bool enable_log_analyze() { return enable_log_analyze_; }

  inline bool enable_log_ggs() { return enable_log_ggs_; }

  inline int  command_id() { return command_id_; }

  inline class RegisterWindow* register_window() {
    return register_window_;
  }
  inline class CodeWindow* code_window() {
    return code_window_;
  }
  inline class MemoryWindow* memory_window() {
    return memory_window_;
  }
  inline class BreakPointWindow* breakpoint_window() {
    return breakpoint_window_;
  }
  inline class TraceWindow* callstack_window() {
    return trace_window_;
  }
  inline class ReferenceWindow* reference_window() {
    return reference_window_;
  }
  inline class PatchWindow* patch_window() {
    return patch_window_;
  }
  inline class ProfilerWindow* profiler_window() {
    return profiler_window_;
  }

  inline class CodeBreakPoint* step_breakpoint() { return &step_breakpoint_; }

  class ModuleInfo* module(int p_idx);
  int               module_count();
  void              module_insert(int p_idx, class ModuleInfo* p_obj);

  CallStackEntry* callstack(int p_idx) { return callstack_stk_[p_idx]; }
  int             callstack_count() { return callstack_stk_.size(); }

  JumpLogEntry* jumplog(int p_idx) { return jumplog_ary_[p_idx]; }
  int           jumplog_count() { return jumplog_ary_.size(); }

  inline bool without_debugger_inspection() { return without_debugger_inspection_; }

  inline bool enable_profiler() { return enable_profiler_; }
  inline u32  profiler_start() { return profiler_start_; }
  inline u32  profiler_end() { return profiler_end_; }

  inline void set_actual_size_screen(bool p_val) { actual_size_screen_ = p_val; }
  inline void set_enable_log_cpu(bool p_val) { enable_log_cpu_ = p_val; }
  inline void set_enable_log_gs(bool p_val) { enable_log_gs_ = p_val; }
  inline void set_enable_log_mem(bool p_val) { enable_log_mem_ = p_val; }
  inline void set_enable_log_elf(bool p_val) { enable_log_elf_ = p_val; }
  inline void set_enable_log_dma(bool p_val) { enable_log_dma_ = p_val; }
  inline void set_enable_log_rec(bool p_val) { enable_log_rec_ = p_val; }
  inline void set_enable_log_app(bool p_val) { enable_log_app_ = p_val; }
  inline void set_enable_log_ddb(bool p_val) { enable_log_ddb_ = p_val; }
  inline void set_enable_log_symbol(bool p_val) { enable_log_symbol_ = p_val; }
  inline void set_enable_log_analyze(bool p_val) { enable_log_analyze_ = p_val; }

  inline void set_enable_log_ggs(bool p_val) { enable_log_ggs_ = p_val; }

  inline void set_command_id(int p_val) { command_id_ = p_val; }

  inline void set_without_debugger_inspection(bool p_val) { without_debugger_inspection_ = p_val; }

  inline void set_enable_profiler(bool p_val) { enable_profiler_ = p_val; }
  inline void set_profiler_start(u32 p_val) { profiler_start_ = p_val; }
  inline void set_profiler_end(u32 p_val) { profiler_end_ = p_val; }

  // -----------------------------
  // debug database access
  s8* get_current_ddb_name();
  DebugDatabase* get_current_ddb();

  //   -----------------------------
  //   breakpoint methods
  int        get_breakpoint_count();
  BreakPointBase*  get_breakpoint(int p_idx);
  int        get_breakpoint_idx(u32 p_va, int* p_idx, bool p_cur_only);
  void      add_breakpoint(BreakPointBase* p_data);
  void      delete_breakpoint(int p_idx);

  //   -----------------------------
  //   comment methods
  int        get_comment_count();
  Comment*    get_comment(int p_idx);
  int        get_comment_idx(u32 p_va);
  void      add_comment(Comment* p_data);
  void      delete_comment(int p_idx);
  Comment*    find_comment(u32 p_va);

  //   -----------------------------
  //   code label methods
  int        get_codelabel_count();
  Label*      get_codelabel(int p_idx);
  int        get_codelabel_idx(u32 p_va);
  void      add_codelabel(Label* p_data);
  void      delete_codelabel(int p_idx);
  Label*      find_codelabel(u32 p_va);

  int       getMemLabelCount();
  MemLabel* getMemLabel(int p_idx);
  int       getMemLabelIdx(u32 p_va, int* p_idx);
  void      addMemLabel(MemLabel* p_data);
  void      deleteMemLabel(int p_idx);
  MemLabel* findMemLabel(u32 p_va);

  int         getPatchCount();
  PatchEntry* getPatch(int p_idx);
  void        getPatchIdx(PatchEntry* p_entry, int* p_idx);
  void        addPatch(PatchEntry* p_data);
  void        deletePatch(int p_idx);
  PatchEntry* findPatch(u32 p_va, u32 p_size, bool p_ignore_disable);
  bool        unionPatch(PatchEntry* p_entry);

  int           getMemDumpLabelCount();
  MemDumpLabel* getMemDumpLabel(int p_idx);
  void          addMemDumpLabel(MemDumpLabel* p_data);
  void          deleteMemDumpLabel(int p_idx);

  int           getProfileCount();
  ProfileEntry* getProfile(int p_idx);

private:
  bool getPartDdbIdx(int p_global_idx, int* p_array_idx,
    int* p_local_idx, int(DebugDatabase::*p_count_proc)());
  bool getWholeDdbIdx(int p_array_idx, int p_local_idx,
    int* p_global_idx, int(DebugDatabase::*p_count_proc)());

  // スクリーン実寸表示するか？
  bool  actual_size_screen_;

  // 各種ログを出力するか？
  bool  enable_log_cpu_;
  bool  enable_log_gs_;
  bool  enable_log_mem_;
  bool  enable_log_elf_;
  bool  enable_log_dma_;
  bool  enable_log_rec_;
  bool  enable_log_app_;
  bool  enable_log_ddb_;
  bool  enable_log_symbol_;
  bool  enable_log_analyze_;
  bool  enable_log_ggs_;

  // 実行中のデバッガコマンドID
  int   command_id_;

  // パッチの適用状態
  deque<bool> save_patch_state_ary_;

  // 各種デバッグウインドウ
  RegisterWindow*   register_window_;
  CodeWindow*       code_window_;
  MemoryWindow*     memory_window_;
  BreakPointWindow* breakpoint_window_;
  TraceWindow*      trace_window_;
  ReferenceWindow*  reference_window_;
  PatchWindow*      patch_window_;
  ProfilerWindow*   profiler_window_;

  // 等幅フォント
  HFONT fixed_font_;

  TempCodeBreakPoint step_breakpoint_;  // ステップ実行用ブレークポイント

  vector<ModuleInfo*> module_ary_;

  vector<CallStackEntry*> callstack_stk_;

  vector<JumpLogEntry*> jumplog_ary_;

  // デバッガでの実行高速化のためデバッガ監視を無効にして実行する
  bool without_debugger_inspection_;

  // プロファイラ関連
  bool enable_profiler_;
  u32  profiler_start_;
  u32  profiler_end_;

public:
  // デバッグ情報をファイル単位で管理する
  vector<DebugDatabase*> ddb_ary_;
  // 書込み対象のDDBファイルインデックス
  int cur_ddb_idx_;
  // モジュール内の文字列
  hash_map<int, String*> string_map_;

  // 静的解析データ
  hash_map<int, AnalyzeInfo> analyze_map_;

  // 検索結果
  vector<FindItem*> finditem_ary_;

  // ボトルネック
  vector<BottleNeck*> bottle_neck_ary_;

  // マクロリコンパイルコード情報
  // (デバッガでの可視性の向上の為のデータであり、コード実行には関係ない)
  vector<MacroRecInfo*> macro_rec_ary_;

  // シンボルテーブル
  hash_map<int, Symbol*> symbol_map_;

  // ソースファイル(重複有り)
  vector<SourceFileEntry*> src_file_ary_;
  // ソースコード位置情報(アドレス順)
  vector<SourceLocationEntry*> src_loc_ary_addr_;
  // ソースコード位置情報(ソース行順)
  vector<SourceLocationEntry*> src_loc_ary_line_;  
  // ソースコードの内容・スクロール位置等
  vector<SourceFileInfo*> src_file_info_ary_;

  // メモリブレーク・不正なアクセス等でデバッガを停止する
  bool hit_mem_break_;
  u32  hit_mem_break_va_;

// static variable
public:
  static const TypeDefinition TYPE_DEF[];
};

extern Debugger*  g_dbg;
extern unsigned int hash_int(int& p_key);

#endif // _debugger_debugger
