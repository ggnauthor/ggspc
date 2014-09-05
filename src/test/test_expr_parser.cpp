#if defined _UNIT_TEST

/*---------*/
/* include */
/*---------*/
#include "memory/memory.h"
#include "debugger/expr_parser.h"
extern s8 g_last_error[1024];

/*--------*/
/* define */
/*--------*/
#define _round(_flt) (u32)floor((_flt) + 0.5)

/*--------*/
/* global */
/*--------*/
const int  EXPR_ELEM_BUF_SIZE = 1024;

u8      g_data_buf[256];
Expr_Elem  g_expr_elem_buf[EXPR_ELEM_BUF_SIZE];
int      g_elem_count = 0;

/*----------*/
/* function */
/*----------*/
FIXTURE(expr_parser_fix_01);

SETUP(expr_parser_fix_01) {

  g_memory = new Memory();

  memset(g_data_buf, 0, 256);

  variable_delete_all();
  // 非組み込み型の場合、仮想アドレスを指定
  variable_add("int4", 0x100000, Expr_Val::kInt, 4, 1, false);
  variable_add("int2", 0x100004, Expr_Val::kInt, 2, 1, false);
  variable_add("int1", 0x100006, Expr_Val::kInt, 1, 1, false);
  variable_add("uint4", 0x100008, Expr_Val::kUint, 4, 1, false);
  variable_add("uint2", 0x10000c, Expr_Val::kUint, 2, 1, false);
  variable_add("uint1", 0x10000e, Expr_Val::kUint, 1, 1, false);
  variable_add("flt4", 0x100010, Expr_Val::kFloat, 4, 1, false);
  variable_add("ary8", 0x100020, Expr_Val::kUint, 1, 8, false);
  variable_add("point", 0x100030, Expr_Val::kError, 4, 1, false);
  variable_add_member("point", "x", 0, Expr_Val::kInt, 2);
  variable_add_member("point", "y", 2, Expr_Val::kInt, 2);
  variable_add("window", 0x100040, Expr_Val::kError, 16, 1, false);
  variable_add_member("window", "rect", 0, Expr_Val::kError, 16);
  variable_add("vecary", 0x100050, Expr_Val::kError, 8, 2, false);
  variable_add_member("vecary", "x", 0, Expr_Val::kFloat, 4);
  variable_add_member("vecary", "y", 4, Expr_Val::kFloat, 4);
  // 組み込み型の場合、実アドレスを指定
  variable_add("emb_int4", (u32)&g_data_buf[0x00], Expr_Val::kInt, 4, 1, true);
}

TEARDOWN(expr_parser_fix_01) {
  for (int i = 0; i < g_elem_count; i++) {
    free(g_expr_elem_buf[i].str);
  }
  variable_delete_all();

  delete g_memory;
}

BEGIN_TESTF(expr_parser_000, expr_parser_fix_01){
  try {
    int elem_count = lex("1 + 2 * 3 - 4 /(1+1)", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 13);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);
    
    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kInt && result.toi() == 5);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_001, expr_parser_fix_01){
  try {
    *((u32*)VA2PA(0x100008)) = 0x12345678;

    int g_elem_count = lex("uint4 | ~uint4", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(g_elem_count == 4);

    Expr_Elem* root = make_tree(g_expr_elem_buf, g_elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kUint && result.tou() == 0xffffffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_002, expr_parser_fix_01){
  try {
    *((u8*)VA2PA(0x100020)) = 0xff;
    *((u8*)VA2PA(0x100023)) = 0xaa;

    int elem_count = lex("ary8[3] + 0x1000", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 6);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kUint && result.tou() == 0x000010aa);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_003, expr_parser_fix_01){
  try {
    *((u32*)VA2PA(0x100000)) = 0x120000;
    *((u16*)VA2PA(0x100004)) = 0x3456;
    *((u8* )VA2PA(0x100006)) = 0xff;

    int elem_count = lex("int4 + int2 + int1", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 5);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);
    
    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kInt && result.toi() == 0x123455);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_004, expr_parser_fix_01){
  try {
    *((u32*)VA2PA(0x100008)) = 0x120000;
    *((u16*)VA2PA(0x10000c)) = 0x3456;
    *((u8* )VA2PA(0x10000e)) = 0xff;

    int elem_count = lex("uint4 + uint2 + uint1", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 5);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);
    
    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kUint && result.tou() == 0x123555);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_005, expr_parser_fix_01){
  try {
    *((u8*)VA2PA(0x100030)) = 64;
    *((u8*)VA2PA(0x100032)) = 63;

    int elem_count = lex("point.x + point.y", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 7);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kInt && result.toi() == 127);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_006, expr_parser_fix_01){
  try {
    int elem_count = lex("13.25 + 25.33 + 100", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 5);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kFloat && result.tof() == 138.58f);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_007, expr_parser_fix_01){
  try {
    *((u8*)VA2PA(0x100020)) = 34;

    // 配列に要素指定しない場合は[0]と同義
    int elem_count = lex("ary8", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 1);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kUint && result.tou() == 34);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_008, expr_parser_fix_01){
  try {
    // 配列に要素指定しない場合は[0]と同義
    int elem_count = lex("&uint4", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 2);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kUint && result.tou() == 0x100008);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_009, expr_parser_fix_01){
  try {
    int elem_count = lex("&ary8[5]", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 5);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kUint && result.tou() == 0x100025);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_010, expr_parser_fix_01){
  try {
    int elem_count = lex("&ary8", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 2);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kUint && result.tou() == 0x100020);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_011, expr_parser_fix_01){
  try {
    int elem_count = lex("&point.y", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 4);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kUint && result.tou() == 0x100032);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_012, expr_parser_fix_01){
  try {
    *((float*)VA2PA(0x10005c)) = 1.2345f;

    int elem_count = lex("vecary[1].y", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 6);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kFloat && result.tof() == 1.2345f);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_013, expr_parser_fix_01){
  try {
    *((u32*  )VA2PA(0x100008)) = 1;
    *((float*)VA2PA(0x100058)) = 4.4345f;

    int elem_count = lex("vecary[(uint4 << 1) - 1].x", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 12);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kFloat && result.tof() == 4.4345f);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_014, expr_parser_fix_01){
  try {
    *((u32*)VA2PA(0x100030)) = 0xc0;
    *((u32*)VA2PA(0x100032)) = 0x57;

    int elem_count = lex("point.x | point.y", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 7);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kInt && result.toi() == 0xd7);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_015, expr_parser_fix_01){
  try {
    *((u32*)&g_data_buf[0x00]) = 0x12345678;

    int elem_count = lex("emb_int4", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 1);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kInt && result.toi() == 0x12345678);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_016, expr_parser_fix_01){
  try {
    *((u32*)VA2PA(0x100000)) = 0x12345678;

    int elem_count = lex("*0x100000", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 2);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(g_last_error[0] == '\0');
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kUint && result.tou() == 0x12345678);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// エラーケース

BEGIN_TESTF(expr_parser_100, expr_parser_fix_01){
  try {
    int elem_count = lex("undefined", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 1);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kError);
    WIN_ASSERT_TRUE(strcmp(g_last_error, "変数 \'undefined\' は未定義です。") == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_101, expr_parser_fix_01){
  try {
    int elem_count = lex("point.z", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 3);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kError);
    WIN_ASSERT_TRUE(strcmp(g_last_error, "メンバ \'z\' は未定義です。") == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_102, expr_parser_fix_01){
  try {
    int elem_count = lex("point", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 1);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kError);
    WIN_ASSERT_TRUE(strcmp(g_last_error, "変数 \'point\' は演算に使用できません。") == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_103, expr_parser_fix_01){
  try {
    int elem_count = lex("window.rect", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 3);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kError);
    WIN_ASSERT_TRUE(strcmp(g_last_error, "メンバ \'rect\' は演算に使用できません。") == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_104, expr_parser_fix_01){
  try {
    int elem_count = lex("undefined[0]", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 4);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kError);
    WIN_ASSERT_TRUE(strcmp(g_last_error, "配列 \'undefined\' は未定義です。") == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_105, expr_parser_fix_01){
  try {
    int elem_count = lex("vecary[2].x", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 6);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kError);
    WIN_ASSERT_TRUE(strcmp(g_last_error, "配列 \'vecary\' の境界を越えてアクセスしました。index = 2") == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_106, expr_parser_fix_01){
  try {
    int elem_count = lex("&emb_int4", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 2);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kError);
    WIN_ASSERT_TRUE(strcmp(g_last_error, "組み込み変数 \'emb_int4\' のアドレスは取得できません。") == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(expr_parser_107, expr_parser_fix_01){
  try {
    int elem_count = lex("&123", g_expr_elem_buf, EXPR_ELEM_BUF_SIZE);
    WIN_ASSERT_TRUE(elem_count == 2);

    Expr_Elem* root = make_tree(g_expr_elem_buf, elem_count);

    Expr_Val result = calculate(root);
    WIN_ASSERT_TRUE(result.m_type == Expr_Val::kError);
    WIN_ASSERT_TRUE(strcmp(g_last_error, "アドレスを取得できません。") == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
