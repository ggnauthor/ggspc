#include "main.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/debug_database.h"
#include "debugger/patch_entry.h"

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

DebugDatabase::DebugDatabase() {
}

DebugDatabase::~DebugDatabase() {
  clearBreakPoint();
  clearComment();
  clearCodeLabel();
  clearMemLabel();
  clearMemDumpLabel();
  clearPatch();
  clearProfile();
}

void DebugDatabase::create(const s8* p_ddb_path) {
  strcpy(file_name_, p_ddb_path);
}

void DebugDatabase::load(const s8* p_ddb_path) {

  strcpy(file_name_, p_ddb_path);

  FILE* fp = fopen(p_ddb_path, "r");
  if (fp == NULL) return;

  setPriority(0);
  clearBreakPoint();
  clearComment();
  clearCodeLabel();
  clearMemLabel();
  clearMemDumpLabel();
  clearPatch();
  clearProfile();

  s8 fbuf[4096];
  while (fgets(fbuf, 4096, fp)) {
    size_t len = strlen(fbuf);
    fbuf[len - 1] = '\0';

    const int kMaxElement = 1024;
    char *elem[kMaxElement];
    char* tmpstr = fbuf;
    for (int i = 0; i < kMaxElement; i++) {
      elem[i] = ds_util::strdiv(tmpstr, ':', '\"');
      if (*tmpstr == '\0') break;
    }

    ds_util::strtrim(elem[0]);
    if (strcmp(elem[0], "comment") == 0) {
      u32 va;
      ds_util::str2hex((char*)&va, 4, elem[1], true);

      s8 comment_str[1024];
      ds_util::strreplace(comment_str, 1024, elem[2] + 1, strlen(elem[2]) - 2, "\"\"", "\"");
      addComment(new Comment(this, va, comment_str));
    } else if (strcmp(elem[0], "label") == 0) {
      u32 va;
      ds_util::str2hex((char*)&va, 4, elem[1], true);

      s8 label_str[1024];
      ds_util::strreplace(label_str, 1024, elem[2] + 1, strlen(elem[2]) - 2, "\"\"", "\"");

      Label* label = new Label(this, va, label_str);
      addCodeLabel(label);
    } else if (strcmp(elem[0], "memlbl") == 0) {
      u32 va;
      ds_util::str2hex((char*)&va, 4, elem[1], true);

      s8 label[1024];
      ds_util::strreplace(label, 1024, elem[2] + 1, strlen(elem[2]) - 2, "\"\"", "\"");

      MemLabel* mem_label = new MemLabel(this, va, label);

      s8 comment[1024];
      ds_util::strreplace(comment, 1024, elem[3] + 1, strlen(elem[3]) - 2, "\"\"", "\"");

      mem_label->set_comment(comment);

      mem_label->set_count(atoi(elem[4]));

      int elem_count = atoi(elem[5]);
      for (int i = 0; i < elem_count; i++) {
        if (9 + i * 3 >= kMaxElement) break;
        int type = atoi(elem[6 + i * 3]);
        int size = atoi(elem[7 + i * 3]);

        s8 elem_label[1024];
        ds_util::strreplace(elem_label, 1024, elem[8 + i * 3] + 1, strlen(elem[8 + i * 3]) - 2, "\"\"", "\"");

        mem_label->AddElement(type, size, elem_label);
      }
      mem_label_ary_.push_back(mem_label);
    } else if (strcmp(elem[0], "codebrk") == 0) {
      u32 va;

      ds_util::str2hex((char*)&va, 4, elem[1], true);

      CodeBreakPoint* code_break = new CodeBreakPoint(this, va);
      code_break->set_enable(strtol(elem[2], NULL, 10) != 0);

      s8 expr[1024];
      ds_util::strreplace(expr, 1024, elem[3] + 1, strlen(elem[3]) - 2, "\"\"", "\"");
      code_break->set_expr(expr);

      breakpoint_ary_.push_back(code_break);
    } else if (strcmp(elem[0], "membrk") == 0) {
      u32 va;
      ds_util::str2hex((char*)&va, 4, elem[1], true);
      
      MemoryBreakPoint *mem_break = new MemoryBreakPoint(this, va);
      mem_break->set_read_enable(strtol(elem[4], NULL, 10) == 1);
      mem_break->set_write_enable(strtol(elem[5], NULL, 10) == 1);
      mem_break->set_length(strtol(elem[2], NULL, 10));
      mem_break->set_enable(strtol(elem[3], NULL, 10) != 0);

      s8 expr[1024];
      ds_util::strreplace(expr, 1024, elem[6] + 1, strlen(elem[6]) - 2, "\"\"", "\"");
      mem_break->set_expr(expr);

      breakpoint_ary_.push_back(mem_break);
    } else if (strcmp(elem[0], "dumplbl") == 0) {
      s8 label[1024];
      ds_util::strreplace(label, 1024, elem[1] + 1, strlen(elem[1]) - 2, "\"\"", "\"");

      u32 start_va  = strtoul(elem[2], NULL, 16);
      u32 end_va    = strtoul(elem[3], NULL, 16);
      memdump_label_ary_.push_back(new MemDumpLabel(label, start_va, end_va));
    } else if (strcmp(elem[0], "patch") == 0) {
      u32 va;
      ds_util::str2hex((char*)&va, 4, elem[1], true);

      int  size = atoi(elem[2]);
      bool state = atoi(elem[3]) != 0;

      s8 name[1024];
      ds_util::strreplace(name, 1024, elem[4] + 1, strlen(elem[4]) - 2, "\"\"", "\"");

      s8* buf = new s8[size];
      ds_util::str2hex(buf, size, elem[5], false);

      // 一旦すべてのパッチを無効に
      g_dbg->allPatchDisableWithSave();

      PatchEntry* patch = new PatchEntry(va, size, state, buf);
      patch->set_name(name);
      patch_ary_.push_back(patch);

      // パッチを復元
      g_dbg->patchRestore();

      delete[] buf;
    } else if (strcmp(elem[0], "priority") == 0) {
      setPriority(atoi(elem[1]));
    } else if (strcmp(elem[0], "curddb") == 0) {
      if (atoi(elem[1]) != 0) {
        // このDDBをカレントに設定
        g_dbg->cur_ddb_idx_ = g_dbg->ddb_ary_.size();
      }
    }
  }
}

void DebugDatabase::save() {
  // ディレクトリがなければ作成
  s8 dir_path[_MAX_PATH];
  sprintf_s(dir_path, _MAX_PATH, "%s/data/%s", g_base_dir, g_elf_name);
  CreateDirectory(dir_path, NULL);

  FILE* fp = fopen(file_name_, "w");
  if (fp == NULL) return;

  fprintf(fp, "priority:%d:\n", priority_);
  fprintf(fp, "curddb:%d:\n", g_dbg->getDDBIndex(this) == g_dbg->cur_ddb_idx_);

  for (u32 i = 0; i < comment_ary_.size(); i++) {
    s8 comment[1024];
    ds_util::strreplace(comment, 1024, comment_ary_[i]->text(), strlen(comment_ary_[i]->text()), "\"", "\"\"");
    fprintf(fp, "comment:%08x:\"%s\"\n", comment_ary_[i]->va(), comment);
  }

  for (u32 i = 0; i < code_label_ary_.size(); i++) {
    s8 label[1024];
    ds_util::strreplace(label, 1024, code_label_ary_[i]->text(), strlen(code_label_ary_[i]->text()), "\"", "\"\"");
    fprintf(fp, "label  :%08x:\"%s\"\n", code_label_ary_[i]->va(), label);
  }

  for (u32 i = 0; i < mem_label_ary_.size(); i++) {
    s8 label[1024];
    ds_util::strreplace(label, 1024, mem_label_ary_[i]->text(), strlen(mem_label_ary_[i]->text()), "\"", "\"\"");
    s8 comment[1024];
    ds_util::strreplace(comment, 1024, mem_label_ary_[i]->comment(), strlen(mem_label_ary_[i]->comment()), "\"", "\"\"");

    fprintf(fp, "memlbl :%08x:\"%s\":\"%s\":%d:%d",
      mem_label_ary_[i]->va(),
      label,
      comment,
      mem_label_ary_[i]->count(),
      mem_label_ary_[i]->element_size());
    for (int j = 0; j < mem_label_ary_[i]->element_size(); j++) {
      MemLabelElement* elem = mem_label_ary_[i]->element(j);
      fprintf(fp, ":%d:%d:\"%s\"", elem->type, elem->size, elem->label ? elem->label : "");
    }
    fprintf(fp, "\n");
  }

  for (u32 i = 0; i < breakpoint_ary_.size(); i++) {
    if (breakpoint_ary_[i]->type() == BreakPointBase::kType_Code) {
      CodeBreakPoint* codeBreak = dynamic_cast<CodeBreakPoint*>(breakpoint_ary_[i]);
      fprintf(fp, "codebrk:%08x:%d:\"%s\"\n",
        codeBreak->va(),
        codeBreak->enable(),
        codeBreak->expr());
    } else if (breakpoint_ary_[i]->type() == BreakPointBase::kType_Memory) {
      MemoryBreakPoint* memBreak = dynamic_cast<MemoryBreakPoint*>(breakpoint_ary_[i]);
      fprintf(fp, "membrk :%08x:%d:%d:%d:%d:\"%s\"\n",
        memBreak->va(),
        memBreak->length(),
        memBreak->enable(),
        memBreak->read_enable(),
        memBreak->write_enable(),
        memBreak->expr());
    }
  }

  for (u32 i = 0; i < memdump_label_ary_.size(); i++) {
    s8 label[1024];
    ds_util::strreplace(label, 1024, memdump_label_ary_[i]->label(), strlen(memdump_label_ary_[i]->label()), "\"", "\"\"");
    fprintf(fp, "dumplbl:\"%s\":%08x:%08x\n", label, memdump_label_ary_[i]->start_va(), memdump_label_ary_[i]->end_va());
  }

  for (u32 i = 0; i < patch_ary_.size(); i++) {
    s8 name[1024];
    ds_util::strreplace(name, 1024, patch_ary_[i]->name(), strlen(patch_ary_[i]->name()), "\"", "\"\"");
    fprintf(fp, "patch  :%08x:%d:%d:\"%s\":", patch_ary_[i]->va(), patch_ary_[i]->size(), patch_ary_[i]->state(), name);
    for (int j = 0; j < patch_ary_[i]->size(); j++) {
      fprintf(fp, "%02x", *(patch_ary_[i]->new_data() + j) & 0xff);
    }
    fprintf(fp, "\n");
  }

  fclose(fp);
}

int DebugDatabase::getBreakPointIdx(u32 p_va, int* p_idx) {
  // p_vaを跨ぐブレークポイントが存在する場合、
  // アドレスの先頭からのオフセットを返す
  // 存在しない場合は-1を返す
  for (u32 i = 0; i < breakpoint_ary_.size(); i++) {
    u32 va1 = VA2PA(p_va);
    u32 va2 = VA2PA(breakpoint_ary_[i]->va());
    if (va1 >= va2 && va1 < va2 + breakpoint_ary_[i]->length()) {
      if (p_idx) *p_idx = i;
      return va1 - va2;
    }
  }
  if (p_idx) *p_idx = -1;
  return -1;
}

void DebugDatabase::clearBreakPoint() {
  for (vector<BreakPointBase*>::iterator itr = breakpoint_ary_.begin();
      itr != breakpoint_ary_.end();
      ++itr) {
    delete *itr;
  }
  breakpoint_ary_.clear();
}

int DebugDatabase::getCommentIdx(u32 p_va) {
  for (u32 i = 0; i < comment_ary_.size(); i++) {
    if (comment_ary_[i]->va() == p_va) {
      return i;
    }
  }
  return -1;
}

void DebugDatabase::clearComment() {
  for (vector<Comment*>::iterator itr = comment_ary_.begin();
      itr != comment_ary_.end();
      ++itr) {
    delete *itr;
  }
  comment_ary_.clear();
  comment_map_.clear();
}

int DebugDatabase::getCodeLabelIdx(u32 p_va) {
  for (u32 i = 0; i < code_label_ary_.size(); i++) {
    if (code_label_ary_[i]->va() == p_va) {
      return i;
    }
  }
  return -1;
}

void DebugDatabase::clearCodeLabel() {
  for (vector<Label*>::iterator itr = code_label_ary_.begin();
      itr != code_label_ary_.end();
      ++itr) {
    delete *itr;
  }
  code_label_ary_.clear();
  code_label_map_.clear();
}

MemLabel* DebugDatabase::findMemLabel(u32 p_va) {
  for (u32 i = 0; i < mem_label_ary_.size(); i++) {
    if (mem_label_ary_[i]->va() == p_va) {
      return mem_label_ary_[i];
    }
  }
  return NULL;
}

int DebugDatabase::getMemLabelIdx(u32 p_va, int* p_idx) {
  // p_vaを跨ぐメモリラベルが存在する場合、
  // アドレスの先頭からのオフセットを返す
  // 存在しない場合は-1を返す。
  for (u32 i = 0; i < mem_label_ary_.size(); i++) {
    if (p_va >= mem_label_ary_[i]->va() &&
        p_va <  mem_label_ary_[i]->va() + mem_label_ary_[i]->GetSize()) {
      if (p_idx) *p_idx = i;
      return p_va - mem_label_ary_[i]->va();
    }
  }
  if (p_idx) *p_idx = -1;
  return -1;
}

void DebugDatabase::clearMemLabel() {
  for (vector<MemLabel*>::iterator itr = mem_label_ary_.begin();
      itr != mem_label_ary_.end();
      ++itr) {
    delete *itr;
  }
  mem_label_ary_.clear();
}

void DebugDatabase::clearMemDumpLabel() {
  for (vector<MemDumpLabel*>::iterator itr = memdump_label_ary_.begin();
      itr != memdump_label_ary_.end();
      ++itr) {
    delete *itr;
  }
  memdump_label_ary_.clear();
}

void DebugDatabase::addPatch(PatchEntry* p_data) {
  patch_ary_.push_back(p_data);
}

void DebugDatabase::deletePatch(int p_idx) {
  delete patch_ary_[p_idx];
  patch_ary_.erase(&patch_ary_[p_idx]);
}

PatchEntry* DebugDatabase::findPatch(u32 p_va, u32 p_size, bool p_ignore_disable) {
  for (u32 i = 0; i < patch_ary_.size(); i++) {
    if (p_ignore_disable && patch_ary_[i]->state() == false) continue;

    s64 end = (s64)p_va + p_size;
    if (p_va < patch_ary_[i]->va() &&
        end <= patch_ary_[i]->va()) {
      continue;
    }

    if (p_va >= patch_ary_[i]->va() + patch_ary_[i]->size() &&
        end  >= patch_ary_[i]->va() + patch_ary_[i]->size()) {
      continue;
    }
    return patch_ary_[i];
  }
  return NULL;
}

void DebugDatabase::clearPatch() {
  for (vector<PatchEntry*>::iterator itr = patch_ary_.begin();
      itr != patch_ary_.end();
      ++itr) {
    delete *itr;
  }
  patch_ary_.clear();
}

void DebugDatabase::clearProfile() {
  for (vector<ProfileEntry*>::iterator itr = profile_ary_.begin();
      itr != profile_ary_.end();
      ++itr) {
    delete *itr;
  }
  profile_ary_.clear();
}
