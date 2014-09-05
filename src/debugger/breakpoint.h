#ifndef _debugger_breakpoint
#define _debugger_breakpoint

#include "main.h"

struct Expr_Elem;

class BreakPointBase {
public:
  enum EType {
    kType_Code = 0,
    kType_Memory,
  };

  BreakPointBase(class DebugDatabase* p_owner, u32 p_va) {
    m_owner = p_owner;
    va_ = p_va;
    enable_ = true;
    expr_[0] = '\0';
    output_log_ = false;
    is_break_ = false;

    expr_elem_buf_ = NULL;
    expr_elem_count_ = 0;
    expr_tree_root_ = NULL;
  }
  virtual ~BreakPointBase() {
    deleteExprCache();
  }

  virtual void breakOn() = 0;
  virtual void breakOff() = 0;

  bool evaluateExpression();
  void deleteExprCache();

  inline u32       va() const { return va_; }
  inline bool      enable() const { return enable_; }
  inline const s8* expr() const { return expr_; }
  inline bool      output_log() const { return output_log_; }
  inline bool      is_break() const { return is_break_; }

  virtual inline void set_va(u32 p_va) { va_ = p_va; }
  inline void set_expr(char* p_expr) {
    strcpy(expr_, p_expr);
    deleteExprCache();
  }
  inline void set_enable(bool p_enable) {
    enable_ = p_enable;
  }
  inline void set_output_log(bool p_output_log) {
    output_log_ = p_output_log;
  }

  virtual inline EType type() = 0;
  virtual inline u32   length() = 0;

  bool is_cur();

protected:
  // ブレークポイントの仮想アドレス
  u32 va_;
  // ブレークポイントが有効かどうか
  bool enable_;
  // ブレーク条件
  char expr_[1024];
  // ログ出力を行うか？
  bool output_log_;
  // 実際にブレークするか？ステップ実行やランの最初のステップは
  // ブレークを無視する必要があり、ユーザが指定するm_enableとは用途が異なる
  bool is_break_;

  // TODO:データの持ち方について検討する
  Expr_Elem* expr_elem_buf_;
  int        expr_elem_count_;
  Expr_Elem* expr_tree_root_;
  class DebugDatabase* m_owner;
};

class CodeBreakPoint : public BreakPointBase {
public:
  CodeBreakPoint(class DebugDatabase* p_owner, u32 p_va = 0x00000000) :
      BreakPointBase(p_owner, p_va) {
    save_byte_ = 0x00;
  }

  ~CodeBreakPoint() {
    breakOff();
  }

  void breakOn();
  void breakOff();

  inline void set_va(u32 p_va) {
    if (is_break_) breakOff();
    va_ = p_va;
  }

  inline EType type() { return kType_Code; }
  inline u32   length() { return 4; }

private:
  // ブレークポイントで置き換える前の命令
  u8 save_byte_;
};

class TempCodeBreakPoint : public CodeBreakPoint {
public:
  TempCodeBreakPoint(u32 p_va = 0x00000000) : CodeBreakPoint(NULL, p_va) {
  }
};

class MemoryBreakPoint : public BreakPointBase {
public:
  MemoryBreakPoint(class DebugDatabase* p_owner, u32 p_va) :
      BreakPointBase(p_owner, p_va) {
    length_ = 4;
    read_enable_ = true;
    write_enable_ = true;
  }
  ~MemoryBreakPoint() {}
  
  void breakOn();
  void breakOff();

  static u32 checkBreak();

  inline EType type() { return kType_Memory; }
  inline u32   length() { return length_; }
  inline bool  read_enable() { return read_enable_; }
  inline bool  write_enable() { return write_enable_; }

  inline void set_length(u32 p_length) {
    length_ = p_length;
  }
  inline void set_read_enable(bool p_read_enable) {
    read_enable_ = p_read_enable;
  }
  inline void set_write_enable(bool p_write_enable) {
    write_enable_ = p_write_enable;
  }

private:
  u32  length_;
  bool read_enable_;
  bool write_enable_;
};

#endif // _debugger_breakpoint
