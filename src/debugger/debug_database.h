#ifndef _debugger_debug_database
#define _debugger_debug_database

#include "main.h"
#include "debugger/breakpoint.h"
#include "debugger/reference_entry.h"

class PatchEntry;

typedef hash_map<int, Comment*>::iterator   CommentMapItr;
typedef hash_map<int, Comment*>::value_type CommentMapValue;
typedef hash_map<int, Label*>::iterator     LabelMapItr;
typedef hash_map<int, Label*>::value_type   LabelMapValue;

class MemDumpLabel {
public:
  MemDumpLabel(s8* p_label, u32 p_start_va, u32 p_end_va) {
    strncpy(label_, p_label, 256);
    start_va_ = p_start_va;
    end_va_ = p_end_va;
  }
  ~MemDumpLabel() {}

  inline s8* label() { return label_; }
  inline u32 start_va() { return start_va_; }
  inline u32 end_va() { return end_va_; }

private:
  s8  label_[256];
  u32 start_va_;
  u32 end_va_;
};

class ProfileEntry {
public:
  inline u32       va() const { return va_; }
  inline u32       count() const { return count_; }
  inline const s8* label() const { return label_; }
  inline u32       func_time() const { return func_time_; }
  inline u32       fd_time() const { return fd_time_; }

private:
  u32  va_;
  u32  count_;
  s8   label_[256];
  u32  func_time_;
  u32  fd_time_;
};

class DebugDatabase {
// constructor/destructor
public:
  DebugDatabase();
  ~DebugDatabase();

  void create(const s8* p_ddb_path);
  void load(const s8* p_ddb_path);
  void save();

  inline s8* file_name() { return file_name_; }

  inline int getBreakPointCount() {
    return breakpoint_ary_.size();
  }
  inline BreakPointBase* getBreakPoint(int p_idx) {
    return breakpoint_ary_[p_idx];
  }
  inline void addBreakPoint(BreakPointBase* p_data) {
    breakpoint_ary_.push_back(p_data);
  }
  inline void deleteBreakPoint(int p_idx) {
    delete breakpoint_ary_[p_idx];
    breakpoint_ary_.erase(&breakpoint_ary_[p_idx]);
  }
  int getBreakPointIdx(u32 p_va, int* p_idx);
  void clearBreakPoint();

  inline int getCommentCount() {
    return comment_ary_.size();
  }
  inline Comment* getComment(int p_idx) {
    return comment_ary_[p_idx];
  }
  inline void addComment(Comment* p_data) {
    comment_ary_.push_back(p_data);
    comment_map_.insert(CommentMapValue(p_data->va(), p_data));
  }
  inline void deleteComment(int p_idx) {
    comment_map_.erase(comment_ary_[p_idx]->va());
    delete comment_ary_[p_idx];
    comment_ary_.erase(&comment_ary_[p_idx]);
  }
  inline Comment* findComment(u32 p_va) {
    CommentMapItr itr = comment_map_.find(p_va);
    if (itr != comment_map_.end()) {
      return itr->second;
    }
    return NULL;
  }
  int getCommentIdx(u32 p_va);
  void clearComment();

  inline int getCodeLabelCount() {
    return code_label_ary_.size();
  }
  inline Label* getCodeLabel(int p_idx) {
    return code_label_ary_[p_idx];
  }
  inline void addCodeLabel(Label* p_data) {
    code_label_ary_.push_back(p_data);
    code_label_map_.insert(LabelMapValue(p_data->va(), p_data));
  }
  inline void deleteCodeLabel(int p_idx) {
    code_label_map_.erase(code_label_ary_[p_idx]->va());
    delete code_label_ary_[p_idx];
    code_label_ary_.erase(&code_label_ary_[p_idx]);
  }
  inline Label* findCodeLabel(u32 p_va) {
    LabelMapItr itr = code_label_map_.find(p_va);
    if (itr != code_label_map_.end()) {
      return itr->second;
    }
    return NULL;
  }
  int getCodeLabelIdx(u32 p_va);
  void clearCodeLabel();

  inline int getMemLabelCount() {
    return mem_label_ary_.size();
  }
  inline MemLabel* getMemLabel(int p_idx) {
    return mem_label_ary_[p_idx];
  }
  inline void addMemLabel(MemLabel* p_data) {
    mem_label_ary_.push_back(p_data);
  }
  inline void deleteMemLabel(int p_idx) {
    delete mem_label_ary_[p_idx];
    mem_label_ary_.erase(&mem_label_ary_[p_idx]);
  }
  MemLabel* findMemLabel(u32 p_va);
  int getMemLabelIdx(u32 p_va, int* p_idx);
  void clearMemLabel();

  inline int getMemDumpLabelCount() {
    return memdump_label_ary_.size();
  }
  inline MemDumpLabel* getMemDumpLabel(int p_idx) {
    return memdump_label_ary_[p_idx];
  }
  void addMemDumpLabel(MemDumpLabel* p_data) {
    memdump_label_ary_.push_back(p_data);
  }
  void deleteMemDumpLabel(int p_idx) {
    delete memdump_label_ary_[p_idx];
    memdump_label_ary_.erase(&memdump_label_ary_[p_idx]);
  }
  void clearMemDumpLabel();

  inline int getPatchCount() {
    return patch_ary_.size();
  }
  inline PatchEntry* getPatch(int p_idx) {
    return patch_ary_[p_idx];
  }
  void addPatch(PatchEntry* p_data);
  void deletePatch(int p_idx);
  PatchEntry* findPatch(u32 p_va, u32 p_size, bool p_ignore_disable);
  void clearPatch();

  int getProfileCount() { return profile_ary_.size(); }
  ProfileEntry* getProfile(int p_idx) { return profile_ary_[p_idx]; }
  void clearProfile();

  inline u32 getPriority() const {
    return priority_;
  }

  inline void setPriority(u32 p_val) {
    priority_ = p_val;
  }

private:
  // ddbファイル名
  s8 file_name_[1024];
  // ブレークポイント
  vector<BreakPointBase*> breakpoint_ary_;
  // コメント
  vector<Comment*>        comment_ary_;
  // vaマッピング用のハッシュマップ。実体は配列で管理する。
  hash_map<int, Comment*> comment_map_;
  // コードラベル
  vector<Label*>          code_label_ary_;
  // vaマッピング用のハッシュマップ。実体は配列で管理する。
  hash_map<int, Label*>   code_label_map_;
  // メモリラベル
  vector<MemLabel*>       mem_label_ary_;
  // メモリダンプ履歴
  vector<MemDumpLabel*>   memdump_label_ary_;
  // パッチ
  vector<PatchEntry*>     patch_ary_;
  // プロファイラ
  vector<ProfileEntry*>   profile_ary_;
  // 優先度
  u32                     priority_;
};

#endif // _debugger_debug_database
