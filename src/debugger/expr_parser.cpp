#include "main.h"
#include "stlp_wrapper.h"
#include "memory/memory.h"
#include "debugger/expr_parser.h"

#include <assert.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

enum {
  kOp2  = 0x80,
  kOp1  = 0x40,
  kBrktEd  = 0x20,
  kBrktSt  = 0x10,
  kVar  = 0x08,
  kHex  = 0x04,
  kFlt  = 0x02,
  kDec  = 0x01,
};

enum {
  kOpLShift = 0,
  kOpRShift,
  kOpLessEqual,
  kOpGreaterEqual,
  kOpEqual,
  kOpNotEqual,
  kOpAnd,
  kOpOr,
  kOpMember,
  kOpPlus,
  kOpMinus,
  kOpNegative,
  kOpNot,
  kOpPointer,
  kOpAddress,
  kOpMul,
  kOpDiv,
  kOpMod,
  kOpAdd,
  kOpSub,
  kOpLess,
  kOpGreater,
  kOpLogicalAnd,
  kOpLogicalXor,
  kOpLogicalOr,
};

struct Member {
  s8    name[256];
  u32    offset;
  u32    type;
  u32    size;
  Member*  next;
  bool  embeded;  // レジスタ・特殊変数など
};

struct Variable {
  s8    name[256];
  u32    addr;
  u32    type;
  u32    size;
  s32    count;    // 配列要素数
  Member*  member_list;
  bool  embeded;  // レジスタ・特殊変数など
};

struct Operator {
  const s8  str[3];
  const int  len;
  const int  prio;
  const bool  single;  // 単項演算子(前がオブジェクトだとNG)
};

#define DOUBLE_EXPR(_param)  {                                            \
  if (p_a.m_type != Expr_Val::kError && p_b.m_type != Expr_Val::kError) g_last_error[0] = '\0';          \
  Expr_Val ev(max(p_a.m_type, p_b.m_type), max(p_a.m_size, p_b.m_size), 0, NULL, NULL, false, NULL);        \
  if (p_a.m_type == Expr_Val::kInt)    { s32 tmp = p_a.toi() _param p_b.toi(); ev.m_value = *((u32*)&tmp); }  \
  else if (p_a.m_type == Expr_Val::kUint)  { u32 tmp = p_a.tou() _param p_b.tou(); ev.m_value = tmp; }        \
  else if (p_a.m_type == Expr_Val::kFloat){ f32 tmp = p_a.tof() _param p_b.tof(); ev.m_value = *((u32*)&tmp); }  \
  return ev;                                                    \
}

#define DOUBLE_EXPR_FOR_INT_ONLY(_param)  {                                    \
  if (p_a.m_type != Expr_Val::kError && p_b.m_type != Expr_Val::kError) g_last_error[0] = '\0';          \
  Expr_Val ev(max(p_a.m_type, p_b.m_type), max(p_a.m_size, p_b.m_size), 0, NULL, NULL, false, NULL);        \
  if (p_a.m_type == Expr_Val::kInt)    { s32 tmp = p_a.toi() _param p_b.toi(); ev.m_value = *((u32*)&tmp); }  \
  else if (p_a.m_type == Expr_Val::kUint)  { u32 tmp = p_a.tou() _param p_b.tou(); ev.m_value = tmp; }        \
  else if (p_a.m_type == Expr_Val::kFloat){ ev.m_type = Expr_Val::kError; ev.m_value = 0; }            \
  return ev;                                                    \
}

/*--------*/
/* global */
/*--------*/
vector<Variable*>  g_var_list;
s8  g_last_error[1024] = "";

Operator g_operator[] = {
  "<<",2,10, 0,
  ">>",2,10, 0,
  "<=",2,11, 0,
  ">=",2,11, 0,
  "==",2,12, 0,
  "!=",2,12, 0,
  "&&",2,16, 0,
  "||",2,17, 0,
//  "(", 1, 1, 0,  // 括弧は演算子としては扱わない
//  ")", 1, 1, 0,
//  "[", 1, 2, 0,
//  "]", 1, 2, 0,
  ".", 1, 3, 0,
  "+", 1, 4, 1,  // +符号
  "-", 1, 4, 1,  // -符号
  "~", 1, 5, 1,
  "!", 1, 5, 1,
  "*", 1, 6, 1,  // ポインタ
  "&", 1, 7, 1,  // アドレス
  "*", 1, 8, 0,  // 乗算
  "/", 1, 8, 0,
  "%", 1, 8, 0,
  "+", 1, 9, 0,  // 加算
  "-", 1, 9, 0,  // 減算
  "<", 1,11, 0,
  ">", 1,11, 0,
  "&", 1,13, 0,  // 論理積
  "^", 1,14, 0,
  "|", 1,15, 0,
  "",   0, 0, 0,  // 終端
};

/*----------*/
/* function */
/*----------*/
Expr_Val::Expr_Val() {
  m_type  = 0;
  m_size  = 0;
  m_value = 0;
  m_addr  = NULL;
  m_member= NULL;
  m_embeded = false;
}

Expr_Val::Expr_Val(u32 p_type) {
  m_type  = p_type;
  m_size  = 0;
  m_value = 0;
  m_addr  = NULL;
  m_member= NULL;
  m_embeded = false;
}

Expr_Val::Expr_Val(u32 p_type, u32 p_size, u32 p_value, u32 p_addr, Member* p_member, bool p_embeded, s8* p_name) {
  m_type    = p_type;
  m_size    = p_size;
  m_addr    = p_addr;
  m_member  = p_member;
  m_embeded  = p_embeded;
  m_name    = p_name;

  if (m_type == kInt) {
    if (m_size == 1) m_value = p_value & 0xff;
    if (m_size == 2) m_value = p_value & 0xffff;
    if (m_size == 4) m_value = p_value;
  } else if (m_type == kUint) {
    if (m_size == 1) m_value = p_value & 0xff;
    if (m_size == 2) m_value = p_value & 0xffff;
    if (m_size == 4) m_value = p_value;
  } else if (m_type == kFloat) {
    assert(m_size == 4);
    m_value = p_value;
  } else {
    m_value = 0;
  }
}

s32 Expr_Val::toi() {
  if (m_type == kInt) {
    if (m_size == 1) return (s32)*((s8* )&m_value);
    if (m_size == 2) return (s32)*((s16*)&m_value);
    if (m_size == 4) return (s32)*((s32*)&m_value);
  } else if (m_type == kUint) {
    if (m_size == 1) return m_value & 0xff;
    if (m_size == 2) return m_value & 0xffff;
    if (m_size == 4) return m_value;
  } else if (m_type == kFloat) {
    assert(m_size == 4);
    return (s32)*((f32*)&m_value);
  }
  assert(0);
  return 0;
}

u32 Expr_Val::tou() {
  if (m_type == kInt) {
    if (m_size == 1) return m_value & 0xff;
    if (m_size == 2) return m_value & 0xffff;
    if (m_size == 4) return m_value;
  } else if (m_type == kUint) {
    if (m_size == 1) return m_value & 0xff;
    if (m_size == 2) return m_value & 0xffff;
    if (m_size == 4) return m_value;
  } else if (m_type == kFloat) {
    assert(m_size == 4);
    return (u32)*((f32*)&m_value);
  }
  assert(0);
  return 0;
}

f32 Expr_Val::tof() {
  if (m_type == kInt) {
    if (m_size == 1) return (f32)(*((s32*)&m_value) & 0xff);
    if (m_size == 2) return (f32)(*((s32*)&m_value) & 0xffff);
    if (m_size == 4) return (f32)*((s32*)&m_value);
  } else if (m_type == kUint) {
    if (m_size == 1) return (f32)(m_value & 0xff);
    if (m_size == 2) return (f32)(m_value & 0xffff);
    if (m_size == 4) return (f32)m_value;
  } else if (m_type == kFloat) {
    assert(m_size == 4);
    return *((f32*)&m_value);
  }
  assert(0);
  return 0;
}

// ----------------------------------------------------------------------------
int lex(s8* p_str, Expr_Elem* p_data, int p_elem_max) {

  int count = 0;

  bool dec = true;
  bool hex = true;
  bool flt = true;
  bool var = true;
  bool opl = true;

  for (int i = 0; p_str[i] != '\0'; i++) {

    // 後続可能なタイプ
    u8 need;
    if (count > 0) {
      if (p_data[count - 1].type == kBrktSt
      ||  p_data[count - 1].type == kOp1
      ||  p_data[count - 1].type == kOp2) {
        need = kDec | kFlt | kHex | kVar | kBrktSt | kOp1;
      } else if (p_data[count - 1].type == kVar) {
        need = kOp2 | kBrktEd | kBrktSt;
      } else {
        need = kOp2 | kBrktEd;
      }
    } else {
      need = kDec | kFlt | kHex | kVar | kBrktSt | kOp1;
    }

    // 10進整数
    int dec_len = 0;
    if (need & kDec) {
      for (int j = i; p_str[j] != '\0'; j++) {
        if (p_str[j] < '0' || p_str[j] > '9') {
          break;
        }
        dec_len++;
      }
    }
    // 16進整数
    int hex_len = 0;
    if (need & kHex) {
      for (int j = i; p_str[j] != '\0'; j++) {
        if (j == i) {
          if (p_str[j] != '0') {
            break;
          }
          continue;
        } else if (j == i + 1) {
          if (p_str[j] != 'x' && p_str[j] != 'X') {
            break;
          }
          continue;
        } else {
          if ((p_str[j] < '0' || p_str[j] > '9')
          &&  (p_str[j] < 'a' || p_str[j] > 'f')
          &&  (p_str[j] < 'A' || p_str[j] > 'F')) {
            break;
          }
        }
        hex_len++;
      }
    }
    // 浮動少数
    int flt_len = 0;
    if (need & kFlt) {
      int dot = 0;
      for (int j = i; p_str[j] != '\0'; j++) {
        if ((p_str[j] < '0' || p_str[j] > '9')
        &&  p_str[j] != '.') {
          break;
        }
        if (p_str[j] == '.') dot++;
        flt_len++;
      }
      if (dot == 0) flt_len = 0;
    }
    // 変数
    int var_len = 0;
    if (need & kVar) {
      for (int j = i; p_str[j] != '\0'; j++) {
        if (j == i) {
          if ((p_str[j] < 'a' || p_str[j] > 'z')
          &&  (p_str[j] < 'A' || p_str[j] > 'Z')
          &&   p_str[j] != '_') {
            break;
          }
        } else {
          if ((p_str[j] < 'a' || p_str[j] > 'z')
          &&  (p_str[j] < 'A' || p_str[j] > 'Z')
          &&  (p_str[j] < '0' || p_str[j] > '9')
          &&   p_str[j] != '_') {
            break;
          }
        }
        var_len++;
      }
    }
    // 単項演算子
    int  op1_idx = -1;
    int op1_len = 0;
    if (need & kOp1) {
      for (int j = 0; g_operator[j].len; j++) {
        if (g_operator[j].single == false) continue;
        if (op1_len >= g_operator[j].len) continue;

        if (strncmp(&p_str[i], g_operator[j].str, g_operator[j].len) == 0) {
          op1_idx = j;
          op1_len = g_operator[j].len;
        }
      }
    }
    // ２項演算子
    int  op2_idx = -1;
    int op2_len = 0;
    if (need & kOp2) {
      for (int j = 0; g_operator[j].len; j++) {
        if (g_operator[j].single == true) continue;
        if (op2_len >= g_operator[j].len) continue;

        if (strncmp(&p_str[i], g_operator[j].str, g_operator[j].len) == 0) {
          op2_idx = j;
          op2_len = g_operator[j].len;
        }
      }
    }

    // 括弧
    int brkts_len = 0;
    if (need & kBrktSt) {
      if (p_str[i] == '(' || p_str[i] == '[') {
        brkts_len = 1;
      }
    }
    int brkte_len = 0;
    if (need & kBrktEd) {
      if (p_str[i] == ')' || p_str[i] == ']') {
        brkte_len = 1;
      }
    }

    int  type = -1;
    int max_len = 0;
    if (op1_len > op2_len && op1_len > dec_len && op1_len > hex_len && op1_len > flt_len && op1_len > var_len) {
      type = kOp1;
      max_len = op1_len;
    } else if (op2_len > op1_len && op2_len > dec_len && op2_len > hex_len && op2_len > flt_len && op2_len > var_len) {
      type = kOp2;
      max_len = op2_len;
    } else if (dec_len > op2_len && dec_len > op1_len && dec_len > hex_len && dec_len > flt_len && dec_len > var_len) {
      type = kDec;
      max_len = dec_len;
    } else if (hex_len > op1_len && hex_len > dec_len && hex_len > flt_len && hex_len > var_len) {
      type = kHex;
      max_len = hex_len + 2;
    } else if (flt_len > op1_len && flt_len > dec_len && flt_len > hex_len && flt_len > var_len) {
      type = kFlt;
      max_len = flt_len;
    } else if (var_len > op1_len && var_len > dec_len && var_len > hex_len && var_len > flt_len) {
      type = kVar;
      max_len = var_len;
    }
    if (brkts_len == 1) {
      type = kBrktSt;
      max_len = brkts_len;
    }
    if (brkte_len == 1) {
      type = kBrktEd;
      max_len = brkte_len;
    }

    // 最長一致を登録
    if (max_len > 0) {
      p_data[count].type = type;
      if (type == kOp1) {
        p_data[count].op = op1_idx;
      } else if (type == kOp2) {
        p_data[count].op = op2_idx;
      } else {
        p_data[count].op = 0;
      }
      p_data[count].str  = (s8*)malloc(max_len + 1);
      p_data[count].l    = NULL;
      p_data[count].r    = NULL;

      memcpy(p_data[count].str, &p_str[i], max_len);
      p_data[count].str[max_len] = '\0';
      
      count++;
      i += max_len - 1;
    }
  }
  return count;
}

Expr_Elem* make_tree(Expr_Elem* p_elem, int p_elem_count) {
  int min_idx = -1;
  int min_prio = 0;
  int  bracket_depth = 0;

  for (int i = 0; i < p_elem_count; i++) {
    if (p_elem[i].type == kBrktSt) { bracket_depth++; continue; }
    if (p_elem[i].type == kBrktEd) { bracket_depth--; continue; }
    if (bracket_depth > 0) continue;

    if ((p_elem[i].type & (kOp1 | kOp2)) && min_prio < g_operator[p_elem[i].op].prio) {
      min_idx = i;
      min_prio = g_operator[p_elem[i].op].prio;
    }
  }
  if (min_idx != -1) {
    // 演算子がある場合左辺と右辺についてさらにツリー構築
    if (p_elem[min_idx].type == kOp2) {
      p_elem[min_idx].l = make_tree(p_elem, min_idx);  
    }
    p_elem[min_idx].r = make_tree(&p_elem[min_idx + 1], p_elem_count - min_idx - 1);
    return &p_elem[min_idx];
  } else {
    // 演算子がない場合、括弧の処理等があれば行う
    if (p_elem_count >= 3 && p_elem[0].str[0] == '(' && p_elem[p_elem_count - 1].str[0] == ')') {
      // (x)の場合
      return make_tree(&p_elem[1], p_elem_count - 2);
    } else if (p_elem_count >= 4 && p_elem[0].type == kVar && p_elem[1].str[0] == '[' && p_elem[p_elem_count - 1].str[0] == ']') {
      // var[x]の場合
      p_elem[1].l = &p_elem[0];
      p_elem[1].r = make_tree(&p_elem[2], p_elem_count - 3);
      return &p_elem[1];
    } else {
      return p_elem;
    }
  }
}

Expr_Val calc_lshift(Expr_Val& p_a, Expr_Val& p_b)    { DOUBLE_EXPR_FOR_INT_ONLY(<<) }
Expr_Val calc_rshift(Expr_Val& p_a, Expr_Val& p_b)    { DOUBLE_EXPR_FOR_INT_ONLY(>>) }
Expr_Val calc_less_eq(Expr_Val& p_a, Expr_Val& p_b)    { DOUBLE_EXPR(<=) }
Expr_Val calc_greater_eq(Expr_Val& p_a, Expr_Val& p_b)  { DOUBLE_EXPR(>=) }
Expr_Val calc_equal(Expr_Val& p_a, Expr_Val& p_b)    { DOUBLE_EXPR(==) }
Expr_Val calc_not_equal(Expr_Val& p_a, Expr_Val& p_b)  { DOUBLE_EXPR(!=) }
Expr_Val calc_and(Expr_Val& p_a, Expr_Val& p_b)      { DOUBLE_EXPR_FOR_INT_ONLY(&&) }
Expr_Val calc_or(Expr_Val& p_a, Expr_Val& p_b)      { DOUBLE_EXPR_FOR_INT_ONLY(||) }
Expr_Val calc_mul(Expr_Val& p_a, Expr_Val& p_b)      { DOUBLE_EXPR(*) }
Expr_Val calc_div(Expr_Val& p_a, Expr_Val& p_b)      { DOUBLE_EXPR(/) }
Expr_Val calc_mod(Expr_Val& p_a, Expr_Val& p_b)      { DOUBLE_EXPR_FOR_INT_ONLY(%) }
Expr_Val calc_add(Expr_Val& p_a, Expr_Val& p_b)      { DOUBLE_EXPR(+) }
Expr_Val calc_sub(Expr_Val& p_a, Expr_Val& p_b)      { DOUBLE_EXPR(-) }
Expr_Val calc_less(Expr_Val& p_a, Expr_Val& p_b)    { DOUBLE_EXPR(<) }
Expr_Val calc_greater(Expr_Val& p_a, Expr_Val& p_b)    { DOUBLE_EXPR(>) }
Expr_Val calc_log_and(Expr_Val& p_a, Expr_Val& p_b)    { DOUBLE_EXPR_FOR_INT_ONLY(&) }
Expr_Val calc_log_xor(Expr_Val& p_a, Expr_Val& p_b)    { DOUBLE_EXPR_FOR_INT_ONLY(^) }
Expr_Val calc_log_or(Expr_Val& p_a, Expr_Val& p_b)    { DOUBLE_EXPR_FOR_INT_ONLY(|) }

Expr_Val calc_minus(Expr_Val& p_a) {
  if (p_a.m_type != Expr_Val::kError) g_last_error[0] = '\0';
  Expr_Val ev(p_a.m_type, p_a.m_size, 0, NULL, NULL, false, NULL);
  if (p_a.m_type == Expr_Val::kInt)    { s32 tmp = -p_a.toi(); ev.m_value = *((u32*)&tmp); }
  else if (p_a.m_type == Expr_Val::kUint)  { u32 tmp = -(s32)p_a.tou(); ev.m_value = tmp; }
  else if (p_a.m_type == Expr_Val::kFloat){ f32 tmp = -p_a.tof(); ev.m_value = *((u32*)&tmp); }
  return ev;
}

Expr_Val calc_negative(Expr_Val& p_a) {
  if (p_a.m_type != Expr_Val::kError) g_last_error[0] = '\0';
  Expr_Val ev(p_a.m_type, p_a.m_size, 0, NULL, NULL, false, NULL);
  if (p_a.m_type == Expr_Val::kInt)    { s32 tmp = ~p_a.toi(); ev.m_value = *((u32*)&tmp); }
  else if (p_a.m_type == Expr_Val::kUint)  { u32 tmp = ~p_a.tou(); ev.m_value = tmp; }
  else if (p_a.m_type == Expr_Val::kFloat){ ev.m_type = Expr_Val::kError; }
  return ev;
}

Expr_Val calc_not(Expr_Val& p_a) {
  if (p_a.m_type != Expr_Val::kError) g_last_error[0] = '\0';
  Expr_Val ev(p_a.m_type, p_a.m_size, 0, NULL, NULL, false, NULL);
  if (p_a.m_type == Expr_Val::kInt)    { s32 tmp = !p_a.toi(); ev.m_value = *((u32*)&tmp); }
  else if (p_a.m_type == Expr_Val::kUint)  { u32 tmp = !p_a.tou(); ev.m_value = tmp; }
  else if (p_a.m_type == Expr_Val::kFloat){ ev.m_type = Expr_Val::kError; }
  return ev;
}

Expr_Val calc_addr(Expr_Val& p_a) {
  if (p_a.m_name == NULL) {
    sprintf(g_last_error, "アドレスを取得できません。");
    return Expr_Val(Expr_Val::kError);
  }
  if (p_a.m_embeded) {
    sprintf(g_last_error, "組み込み変数 \'%s\' のアドレスは取得できません。", p_a.m_name);
    return Expr_Val(Expr_Val::kError);
  }
  if (p_a.m_type != Expr_Val::kError) g_last_error[0] = '\0';
  return Expr_Val(Expr_Val::kUint, 4, p_a.m_addr, NULL, NULL, false, NULL);
}

Expr_Val calc_pointer(Expr_Val& p_a) {
  if (p_a.m_type != Expr_Val::kError) g_last_error[0] = '\0';
  // 面倒なので強制的に４バイトUintとして解釈する
  return Expr_Val(Expr_Val::kUint, 4, *((u32*)VA2PA(p_a.m_value)), NULL, NULL, false, NULL);
}

Expr_Val mbr2val(Expr_Val p_object, Expr_Elem* p_member) {
  int    type, size, offset;
  bool  embeded;
  Member* member = p_object.m_member;
  if (member == NULL) return Expr_Val(Expr_Val::kError);
  while (member) {
    if (strcmp(member->name, p_member->str) == 0) {
      type  = member->type;
      size  = member->size;
      offset  = member->offset;
      embeded  = member->embeded;
      break;
    }
    member = member->next;
  }
  if (member == NULL) {
    sprintf(g_last_error, "メンバ \'%s\' は未定義です。", p_member->str);
    return Expr_Val(Expr_Val::kError);
  }
  if (type == Expr_Val::kError) {
    sprintf(g_last_error, "メンバ \'%s\' は演算に使用できません。", p_member->str);
    return Expr_Val(Expr_Val::kError);
  }
  u32 addr = p_object.m_addr + offset;
  u32 val;
  if (embeded) {
    val = *((u32*)addr);
  } else {
    val = *((u32*)VA2PA(addr));
  }
  return Expr_Val(type, size, val, addr, NULL, embeded, p_member->str);
}

Expr_Val var2val(Expr_Elem* p_elem) {
  for (u32 i = 0; i < g_var_list.size(); i++) {
    if (strcmp(p_elem->str, g_var_list[i]->name) == 0) {
      
      if (g_var_list[i]->type == Expr_Val::kError) {
        sprintf(g_last_error, "変数 \'%s\' は演算に使用できません。", p_elem->str);
        // この後メンバ参照される場合はエラーにならない
        //return Expr_Val(Expr_Val::kError);
      }
      u32 val;
      if (g_var_list[i]->embeded) {
        val = *((u32*)g_var_list[i]->addr);
      } else {
        val = *((u32*)VA2PA(g_var_list[i]->addr));
      }
      return Expr_Val(g_var_list[i]->type, g_var_list[i]->size, val, g_var_list[i]->addr, g_var_list[i]->member_list, g_var_list[i]->embeded, g_var_list[i]->name);
    }
  }
  sprintf(g_last_error, "変数 \'%s\' は未定義です。", p_elem->str);
  return Expr_Val(Expr_Val::kError);
}

Expr_Val ary2val(Expr_Elem* p_elem, int p_index) {
  assert(p_elem->str[0] == '[');

  for (u32 i = 0; i < g_var_list.size(); i++) {
    if (strcmp(p_elem->l->str, g_var_list[i]->name) == 0) {

      if (p_index < 0 || p_index >= g_var_list[i]->count) {
        sprintf(g_last_error, "配列 \'%s\' の境界を越えてアクセスしました。index = %d", p_elem->l->str, p_index);
        return Expr_Val(Expr_Val::kError);
      }

      u32 addr = g_var_list[i]->addr + g_var_list[i]->size * p_index;
      u32 val;
      if (g_var_list[i]->embeded) {
        val = *((u32*)addr);
      } else {
        val = *((u32*)VA2PA(addr));
      }
      return Expr_Val(g_var_list[i]->type, g_var_list[i]->size, val, addr, g_var_list[i]->member_list, g_var_list[i]->embeded, p_elem->l->str);
    }
  }
  sprintf(g_last_error, "配列 \'%s\' は未定義です。", p_elem->l->str);
  return Expr_Val(Expr_Val::kError);
}

Expr_Val _calculate(Expr_Elem* p_elem) {
  switch (p_elem->type) {
  case kDec:
    return Expr_Val(Expr_Val::kInt, 4, atoi(p_elem->str), NULL, NULL, false, NULL);
  case kHex:
    return Expr_Val(Expr_Val::kUint, 4, strtoul(p_elem->str, NULL, 16), NULL, NULL, false, NULL);
  case kFlt: {
    float val = (float)strtod(p_elem->str, NULL);
    return Expr_Val(Expr_Val::kFloat, 4, *((u32*)&val), NULL, NULL, false, NULL);
    }
  case kVar:
    return var2val(p_elem);
  case kBrktSt:
    return ary2val(p_elem, _calculate(p_elem->r).toi());
  case kOp1:
    switch (p_elem->op) {
    case kOpPlus:
      return _calculate(p_elem->r);
    case kOpMinus:
      return calc_minus(_calculate(p_elem->r));
    case kOpNegative:
      return calc_negative(_calculate(p_elem->r));
    case kOpNot:
      return calc_not(_calculate(p_elem->r));
    case kOpPointer:
      return calc_pointer(_calculate(p_elem->r));
      //return Expr_Val(Expr_Val::kError);
    case kOpAddress:
      return calc_addr(_calculate(p_elem->r));
    }
    break;
  case kOp2:
    switch (p_elem->op) {
    case kOpLShift:
      return calc_lshift(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpRShift:
      return calc_rshift(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpLessEqual:
      return calc_less_eq(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpGreaterEqual:
      return calc_greater_eq(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpEqual:
      return calc_equal(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpNotEqual:
      return calc_not_equal(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpAnd:
      return calc_and(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpOr:
      return calc_or(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpMember:
      return mbr2val(_calculate(p_elem->l), p_elem->r);
    case kOpMul:
      return calc_mul(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpDiv:
      return calc_div(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpMod:
      return calc_mod(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpAdd:
      return calc_add(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpSub:
      return calc_sub(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpLess:
      return calc_less(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpGreater:
      return calc_greater(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpLogicalAnd:
      return calc_log_and(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpLogicalXor:
      return calc_log_xor(_calculate(p_elem->l), _calculate(p_elem->r));
    case kOpLogicalOr:
      return calc_log_or(_calculate(p_elem->l), _calculate(p_elem->r));
    }
    break;
  }
  return Expr_Val(Expr_Val::kError);
}

Expr_Val calculate(Expr_Elem* p_elem) {
  g_last_error[0] = '\0';
  return _calculate(p_elem);
}

void show_parser_error_message() {
  MessageBox(NULL, g_last_error, NULL, NULL);
}

//bool evaluate_expression(s8* p_str) {  
//
//  Expr_Elem elem[1024];
//  int elem_count = lex(p_str, elem, 1024);
//
//  Expr_Elem* root = make_tree(elem, elem_count);
//
//  u32 result = calculate(root);
//
//  for (int i = 0; i < elem_count; i++) {
//    free(elem[i].str);
//  }
//  return result != 0;
//}

void variable_delete_all() {
  for (u32 i = 0; i < g_var_list.size(); i++) {
    Member* member = g_var_list[i]->member_list;
    while (member) {
      Member* tmp = member;
      member = member->next;
      delete tmp;
    }
  }

  for (vector<Variable*>::iterator itr = g_var_list.begin();
      itr != g_var_list.end();
      ++itr) {
    delete *itr;
  }
  g_var_list.clear();
}

void variable_add(const s8* p_name, u32 p_addr, u32 p_type, u32 p_size, u32 p_count, bool p_embeded) {
  Variable* var = new Variable;
  strncpy(var->name, p_name, 256);
  var->addr = p_addr;
  var->type = p_type;
  var->size = p_size;
  var->count= p_count;
  var->member_list = NULL;
  var->embeded = p_embeded;
  g_var_list.push_back(var);
}

void variable_add_member(const s8* p_struct_name, const s8* p_member_name, u32 p_offset, u32 p_type, u32 p_size) {

  int idx = 0;
  for (u32 i = 0; i < g_var_list.size(); i++) {
    if (strcmp(g_var_list[i]->name, p_struct_name) != 0) continue;

    for (Member* mem = g_var_list[i]->member_list; mem; mem = mem->next) {
      if (strcmp(mem->name, p_member_name) == 0) {
        // 既にあれば情報を書き換え
        mem->offset = p_offset;
        mem->type = p_type;
        mem->size = p_size;
        return;
      }
    }
    Member* new_member = new Member;
    strncpy(new_member->name, p_member_name, 256);
    new_member->offset = p_offset;
    new_member->type = p_type;
    new_member->size = p_size;
    new_member->next = g_var_list[i]->member_list;
    new_member->embeded = g_var_list[i]->embeded;
    g_var_list[i]->member_list = new_member;
    break;
  }
}
