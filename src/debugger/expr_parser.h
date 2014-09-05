#ifndef _debugger_exprparser
#define _debugger_exprparser

/*---------*/
/* include */
/*---------*/
#include "main.h"

/*--------*/
/* define */
/*--------*/
enum {
  kErrMsg_NoError = -1,
  kErrMsg_IdentifierNotFound = 0,
  kErrMsg_VariableCannotCalculate,
  kErrMsg_MemberCannotCalculate,
};

struct Expr_Elem {
  u8  type;
  u8  op;
  s8* str;
  Expr_Elem*  l;
  Expr_Elem*  r;
};

class Expr_Val {
// constant definition
public:
  enum {
    kError = -1,
    kUint = 0,
    kInt,
    kFloat,
  };

// constructor/destructor
public:
  Expr_Val();
  Expr_Val(u32 p_type);
  Expr_Val(u32 p_type, u32 p_size, u32 p_value, u32 p_addr, struct Member* p_mbr, bool p_embeded, s8* p_name);

// member fucntion
public:
  s32 toi();
  u32 tou();
  f32 tof();

// member variable
public:
  u32        m_type;
  u32        m_size;
  u32        m_value;
  u32        m_addr;
  struct Member*  m_member;
  bool      m_embeded;
  s8*        m_name;
};

/*----------*/
/* function */
/*----------*/
int lex(s8* p_str, Expr_Elem* p_data, int p_max_elem);
Expr_Elem* make_tree(Expr_Elem* p_elem, int p_elem_count);
Expr_Val calculate(Expr_Elem* p_elem);
void show_parser_error_message();

//bool evaluate_expression(s8* p_str);
void variable_delete_all();
void variable_add(const s8* p_name, u32 p_addr, u32 p_type, u32 p_size, u32 p_count, bool p_embeded);
void variable_add_member(const s8* p_struct_name, const s8* p_member_name, u32 p_offset, u32 p_type, u32 p_size);

#endif // _debugger_exprparser
