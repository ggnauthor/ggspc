#include "main.h"
#include "debugger/debugger.h"
#include "debugger/gui/color_def.h"
#include "debugger/gui/window/register_window.h"

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

RegisterWindow::RegisterWindow() {
  m_dlg = NULL;
  m_gpr_page = NULL;
  m_cop0_page = NULL;
  m_fpr_page = NULL;
  m_ignore_wm_command = false;
  m_user_modified = false;
}

RegisterWindow::~RegisterWindow() {
  if (m_dlg) delete m_dlg;
  if (m_gpr_page) delete m_gpr_page;
  if (m_cop0_page) delete m_cop0_page;
  if (m_fpr_page) delete m_fpr_page;
}

void RegisterWindow::Init() {
  if (m_dlg == NULL) {
    m_dlg = new ds_dialog(IDD_REGISTER, NULL, (DLGPROC)FrameProc, 0);
    m_dlg->setUserData((DWORD)this);
    m_dlg->move(kViewX, kViewY);
    m_dlg->size(kViewW, kViewH);
  }
  if (m_gpr_page == NULL) {
    m_gpr_page = new ds_dialog(IDD_GPR, m_dlg->getHwnd(), (DLGPROC)GPRProc, 0);
    m_gpr_page->setUserData((DWORD)this);
    m_dlg->insertTabPage(IDC_TAB, m_gpr_page, "GPR");
  }
  if (m_cop0_page == NULL) {
    m_cop0_page = new ds_dialog(IDD_COP0, m_dlg->getHwnd(), (DLGPROC)COP0Proc, 0);
    m_cop0_page->setUserData((DWORD)this);
    m_dlg->insertTabPage(IDC_TAB, m_cop0_page, "COP0");
  }
  if (m_fpr_page == NULL) {
    m_fpr_page = new ds_dialog(IDD_FPR, m_dlg->getHwnd(), (DLGPROC)FPRProc, 0);
    m_fpr_page->setUserData((DWORD)this);
    m_dlg->insertTabPage(IDC_TAB, m_fpr_page, "FPR");
  }
  // CPUの状態をコピー
  memcpy(&m_save_r5900, g_cpu, sizeof(R5900REGS));
}

void RegisterWindow::gpr_update() {
  m_ignore_wm_command = true;

  char str[256];
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 4; j++) {
      sprintf(str, "%08x", g_cpu->m_gpr[i].d[j]);
      SetDlgItemText(m_gpr_page->getHwnd(), IDC_ZE0 + i * 4 + j, str);
    }
  }
  for (int j = 0; j < 4; j++) {
    sprintf(str, "%08x", g_cpu->m_hi.d[j]);
    SetDlgItemText(m_gpr_page->getHwnd(), IDC_Hi0 + j, str);

    sprintf(str, "%08x", g_cpu->m_lo.d[j]);
    SetDlgItemText(m_gpr_page->getHwnd(), IDC_Lo0 + j, str);
  }
  sprintf(str, "%08x", g_cpu->m_sa);
  SetDlgItemText(m_gpr_page->getHwnd(), IDC_SA, str);

  sprintf(str, "%08x", g_cpu->m_pc);
  SetDlgItemText(m_gpr_page->getHwnd(), IDC_PC, str);

  ShowWindow(GetParent(m_gpr_page->getHwnd()), SW_SHOW);

  m_ignore_wm_command = false;
}

void RegisterWindow::cop0_update() {
  m_ignore_wm_command = true;

  char str[256];
  for (int i = 0; i < 32; i++)
  {
    sprintf(str, "%08x", g_cpu->m_cop0.cpr[i]);
    SetDlgItemText(m_cop0_page->getHwnd(), IDC_INDEX + i, str);
  }
  ShowWindow(GetParent(m_cop0_page->getHwnd()), SW_SHOW);

  m_ignore_wm_command = false;
}

void RegisterWindow::fpr_update() {
  m_ignore_wm_command = true;

  char str[256];
  for (int i = 0; i < 32; i++) {
    sprintf(str, "%08x", *((u32*)&g_cpu->m_fpu.fpr[i]));
    SetDlgItemText(m_fpr_page->getHwnd(), IDC_FPR0 + i, str);

    sprintf(str, "%f", g_cpu->m_fpu.fpr[i]);
    SetDlgItemText(m_fpr_page->getHwnd(), IDC_FPR_F0 + i, str);
  }
  sprintf(str, "%08x", g_cpu->m_fpu.acc);
  SetDlgItemText(m_fpr_page->getHwnd(), IDC_ACC, str);
  sprintf(str, "%f", g_cpu->m_fpu.acc);
  SetDlgItemText(m_fpr_page->getHwnd(), IDC_ACC_F, str);

  sprintf(str, "%08x", g_cpu->m_fpu.fcr0);
  SetDlgItemText(m_fpr_page->getHwnd(), IDC_FCR0, str);
  sprintf(str, "%08x", g_cpu->m_fpu.fcr31);
  SetDlgItemText(m_fpr_page->getHwnd(), IDC_FCR31, str);

  ShowWindow(GetParent(m_fpr_page->getHwnd()), SW_SHOW);

  InvalidateRect(m_fpr_page->getHwnd(), NULL,1);
  UpdateWindow(m_fpr_page->getHwnd());

  m_ignore_wm_command = false;
}

int RegisterWindow::reg2idc(int p_reg, int p_idx) {
  // レジスタ値をコントロールIDに変換する
  // コントロールIDは整列していること！
  return IDC_ZE0 + p_reg * 4 + p_idx;
}

int RegisterWindow::hwnd2idc(HWND p_hwnd, HWND p_parent) {
  // ウインドウハンドルをコントロールIDに変換する
  if (m_gpr_page->getHwnd() == p_parent || p_parent == NULL) {
#define _gpr_check(_idc)                                                    \
  if (GetDlgItem(m_gpr_page->getHwnd(), _idc##0) == p_hwnd) return _idc##0; \
  if (GetDlgItem(m_gpr_page->getHwnd(), _idc##1) == p_hwnd) return _idc##1; \
  if (GetDlgItem(m_gpr_page->getHwnd(), _idc##2) == p_hwnd) return _idc##2; \
  if (GetDlgItem(m_gpr_page->getHwnd(), _idc##3) == p_hwnd) return _idc##3;

    _gpr_check(IDC_ZE);
    _gpr_check(IDC_AT);
    _gpr_check(IDC_V0);
    _gpr_check(IDC_V1);
    _gpr_check(IDC_A0);
    _gpr_check(IDC_A1);
    _gpr_check(IDC_A2);
    _gpr_check(IDC_A3);
    _gpr_check(IDC_T0);
    _gpr_check(IDC_T1);
    _gpr_check(IDC_T2);
    _gpr_check(IDC_T3);
    _gpr_check(IDC_T4);
    _gpr_check(IDC_T5);
    _gpr_check(IDC_T6);
    _gpr_check(IDC_T7);
    _gpr_check(IDC_S0);
    _gpr_check(IDC_S1);
    _gpr_check(IDC_S2);
    _gpr_check(IDC_S3);
    _gpr_check(IDC_S4);
    _gpr_check(IDC_S5);
    _gpr_check(IDC_S6);
    _gpr_check(IDC_S7);
    _gpr_check(IDC_T8);
    _gpr_check(IDC_T9);
    _gpr_check(IDC_K0);
    _gpr_check(IDC_K1);
    _gpr_check(IDC_GP);
    _gpr_check(IDC_SP);
    _gpr_check(IDC_FP);
    _gpr_check(IDC_RA);
    _gpr_check(IDC_Lo);
    _gpr_check(IDC_Hi);
    if (GetDlgItem(m_gpr_page->getHwnd(), IDC_SA) == p_hwnd) return IDC_SA;
    if (GetDlgItem(m_gpr_page->getHwnd(), IDC_PC) == p_hwnd) return IDC_PC;

#undef _gpr_check
  } else if (m_cop0_page->getHwnd() == p_parent || p_parent == NULL) {
#define _cop0_check(_id)                                             \
  if (GetDlgItem(m_cop0_page->getHwnd(), _id) == p_hwnd) return _id; \

    _cop0_check(IDC_INDEX);
    _cop0_check(IDC_RANDOM);
    _cop0_check(IDC_ENTRYLO0);
    _cop0_check(IDC_ENTRYLO1);
    _cop0_check(IDC_CONTEXT);
    _cop0_check(IDC_PAGEMASK);
    _cop0_check(IDC_WIRED);
    _cop0_check(IDC_RESERVED7);
    _cop0_check(IDC_BADVADDR);
    _cop0_check(IDC_COUNT);
    _cop0_check(IDC_ENTERHI);
    _cop0_check(IDC_COMPARE);
    _cop0_check(IDC_STATUS);
    _cop0_check(IDC_CAUSE);
    _cop0_check(IDC_EPC);
    _cop0_check(IDC_PRID);
    _cop0_check(IDC_CONFIG);
    _cop0_check(IDC_RESERVED17);
    _cop0_check(IDC_RESERVED18);
    _cop0_check(IDC_RESERVED19);
    _cop0_check(IDC_RESERVED20);
    _cop0_check(IDC_RESERVED21);
    _cop0_check(IDC_RESERVED22);
    _cop0_check(IDC_BADPADDR);
    _cop0_check(IDC_BPC);
    _cop0_check(IDC_PCCR);
    _cop0_check(IDC_RESERVED26);
    _cop0_check(IDC_RESERVED27);
    _cop0_check(IDC_TAGLO);
    _cop0_check(IDC_TAGHI);
    _cop0_check(IDC_ERROREPC);
    _cop0_check(IDC_RESERVED31);

#undef _cop0_check
  } else if (m_fpr_page->getHwnd() == p_parent || p_parent == NULL) {
#define _fpr_check(_idx)                                            \
  if (GetDlgItem(m_fpr_page->getHwnd(), IDC_FPR##_idx) == p_hwnd)   \
    return IDC_FPR##_idx;                                           \
  if (GetDlgItem(m_fpr_page->getHwnd(), IDC_FPR_F##_idx) == p_hwnd) \
    return IDC_FPR_F##_idx;

    _fpr_check(0);
    _fpr_check(1);
    _fpr_check(2);
    _fpr_check(3);
    _fpr_check(4);
    _fpr_check(5);
    _fpr_check(6);
    _fpr_check(7);
    _fpr_check(8);
    _fpr_check(9);
    _fpr_check(10);
    _fpr_check(11);
    _fpr_check(12);
    _fpr_check(13);
    _fpr_check(14);
    _fpr_check(15);
    _fpr_check(16);
    _fpr_check(17);
    _fpr_check(18);
    _fpr_check(19);
    _fpr_check(20);
    _fpr_check(21);
    _fpr_check(22);
    _fpr_check(23);
    _fpr_check(24);
    _fpr_check(25);
    _fpr_check(26);
    _fpr_check(27);
    _fpr_check(28);
    _fpr_check(29);
    _fpr_check(30);
    _fpr_check(31);
    if (GetDlgItem(m_fpr_page->getHwnd(), IDC_ACC) == p_hwnd) return IDC_ACC;
    if (GetDlgItem(m_fpr_page->getHwnd(), IDC_ACC_F) == p_hwnd) return IDC_ACC_F;
    if (GetDlgItem(m_fpr_page->getHwnd(), IDC_FCR0) == p_hwnd) return IDC_FCR0;
    if (GetDlgItem(m_fpr_page->getHwnd(), IDC_FCR31) == p_hwnd) return IDC_FCR31;

#undef _fpr_check
  }
  return -1;
}

void RegisterWindow::OnPreExecute() {
  m_user_modified = false;

  // 命令が実行される前の状態を保存する
  memcpy(&m_save_r5900, g_cpu, sizeof(R5900REGS));
}

LRESULT CALLBACK RegisterWindow::FrameProc(
                   HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp) {
  switch (p_msg) {
    case WM_ACTIVATE:
      // モーダルダイアログがあったら制御を切り替えない
      if ((p_wp & 0xffff) != WA_INACTIVE) {
        HWND hwnd = g_dbg->findModalDialog();
        if (hwnd) {
          SetActiveWindow(hwnd);
          break;
        }
      }
      break;
  }
  return FALSE;
}

LRESULT CALLBACK RegisterWindow::GPRProc(
                   HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp) {
  ds_dialog* dlg = (ds_dialog*)GetWindowLong(p_hwnd, DWL_USER);

  RegisterWindow* regwin = dlg ? (RegisterWindow*)dlg->getUserData() : NULL;

  switch (p_msg) {
  case WM_INITDIALOG:
    return TRUE;
  case WM_CTLCOLOREDIT:
#define CHANGE_HIGHLIGHT(_idc, _var, _idx)                          \
  case _idc##_idx:                                                  \
    if (regwin->m_save_r5900._var.d[_idx] != g_cpu->_var.d[_idx]) { \
      SetBkMode((HDC)p_wp, TRANSPARENT);                            \
      return (LRESULT)CreateSolidBrush(COLOR_READ);                 \
    }                                                               \
    break;
#define CHANGE_HIGHLIGHT4(_idc, _var) \
  CHANGE_HIGHLIGHT(_idc, _var, 0)     \
  CHANGE_HIGHLIGHT(_idc, _var, 1)     \
  CHANGE_HIGHLIGHT(_idc, _var, 2)     \
  CHANGE_HIGHLIGHT(_idc, _var, 3)

    switch (regwin->hwnd2idc((HWND)p_lp, regwin->m_gpr_page->getHwnd())) {
    //CHANGE_HIGHLIGHT4(IDC_ZE, m_ze)
    CHANGE_HIGHLIGHT4(IDC_AT, m_at)
    CHANGE_HIGHLIGHT4(IDC_V0, m_v0)
    CHANGE_HIGHLIGHT4(IDC_V1, m_v1)
    CHANGE_HIGHLIGHT4(IDC_A0, m_a0)
    CHANGE_HIGHLIGHT4(IDC_A1, m_a1)
    CHANGE_HIGHLIGHT4(IDC_A2, m_a2)
    CHANGE_HIGHLIGHT4(IDC_A3, m_a3)
    CHANGE_HIGHLIGHT4(IDC_T0, m_t0)
    CHANGE_HIGHLIGHT4(IDC_T1, m_t1)
    CHANGE_HIGHLIGHT4(IDC_T2, m_t2)
    CHANGE_HIGHLIGHT4(IDC_T3, m_t3)
    CHANGE_HIGHLIGHT4(IDC_T4, m_t4)
    CHANGE_HIGHLIGHT4(IDC_T5, m_t5)
    CHANGE_HIGHLIGHT4(IDC_T6, m_t6)
    CHANGE_HIGHLIGHT4(IDC_T7, m_t7)
    CHANGE_HIGHLIGHT4(IDC_S0, m_s0)
    CHANGE_HIGHLIGHT4(IDC_S1, m_s1)
    CHANGE_HIGHLIGHT4(IDC_S2, m_s2)
    CHANGE_HIGHLIGHT4(IDC_S3, m_s3)
    CHANGE_HIGHLIGHT4(IDC_S4, m_s4)
    CHANGE_HIGHLIGHT4(IDC_S5, m_s5)
    CHANGE_HIGHLIGHT4(IDC_S6, m_s6)
    CHANGE_HIGHLIGHT4(IDC_S7, m_s7)
    CHANGE_HIGHLIGHT4(IDC_T8, m_t8)
    CHANGE_HIGHLIGHT4(IDC_T9, m_t9)
    CHANGE_HIGHLIGHT4(IDC_K0, m_k0)
    CHANGE_HIGHLIGHT4(IDC_K1, m_k1)
    CHANGE_HIGHLIGHT4(IDC_GP, m_gp)
    CHANGE_HIGHLIGHT4(IDC_SP, m_sp)
    CHANGE_HIGHLIGHT4(IDC_FP, m_fp)
    CHANGE_HIGHLIGHT4(IDC_RA, m_ra)
    CHANGE_HIGHLIGHT4(IDC_Lo, m_lo)
    CHANGE_HIGHLIGHT4(IDC_Hi, m_hi)
    case IDC_SA:
      if (regwin->m_save_r5900.m_sa != g_cpu->m_sa) {
        SetBkMode((HDC)p_wp, TRANSPARENT);
        return (LRESULT)CreateSolidBrush(COLOR_READ);
      }
      break;
    case IDC_PC:
      if (regwin->m_save_r5900.m_pc != g_cpu->m_pc) {
        SetBkMode((HDC)p_wp, TRANSPARENT);
        return (LRESULT)CreateSolidBrush(COLOR_READ);
      }
      break;
    }
#undef CHANGE_HIGHLIGHT4
#undef CHANGE_HIGHLIGHT
    return NULL_BRUSH;
  case WM_COMMAND:
    if (regwin->m_ignore_wm_command == false) {
      int id    = p_wp & 0xffff;
      int notify  = (p_wp >> 16) & 0xffff;
      int hwnd  = p_lp;
      if (notify == EN_CHANGE) {
        char  str[256];
        GetDlgItemText(p_hwnd, id, str, 256);
        u32 value = ds_util::atox(str, 16);

#define SET_REG_VALUE4(_idc, _var)               \
  case _idc##0: g_cpu->_var.d[0] = value; break; \
  case _idc##1: g_cpu->_var.d[1] = value; break; \
  case _idc##2: g_cpu->_var.d[2] = value; break; \
  case _idc##3: g_cpu->_var.d[3] = value; break;

        if (regwin->m_user_modified == false) {
          // 命令実行後のユーザによる初回の値変更のタイミングでCPUの状態をコピー
          memcpy(&regwin->m_save_r5900, g_cpu, sizeof(R5900REGS));
          regwin->m_user_modified = true;
        }

        switch (id) {
          //SET_REG_VALUE4(IDC_ZE, ze)
          SET_REG_VALUE4(IDC_AT, m_at)
          SET_REG_VALUE4(IDC_V0, m_v0)
          SET_REG_VALUE4(IDC_V1, m_v1)
          SET_REG_VALUE4(IDC_A0, m_a0)
          SET_REG_VALUE4(IDC_A1, m_a1)
          SET_REG_VALUE4(IDC_A2, m_a2)
          SET_REG_VALUE4(IDC_A3, m_a3)
          SET_REG_VALUE4(IDC_T0, m_t0)
          SET_REG_VALUE4(IDC_T1, m_t1)
          SET_REG_VALUE4(IDC_T2, m_t2)
          SET_REG_VALUE4(IDC_T3, m_t3)
          SET_REG_VALUE4(IDC_T4, m_t4)
          SET_REG_VALUE4(IDC_T5, m_t5)
          SET_REG_VALUE4(IDC_T6, m_t6)
          SET_REG_VALUE4(IDC_T7, m_t7)
          SET_REG_VALUE4(IDC_S0, m_s0)
          SET_REG_VALUE4(IDC_S1, m_s1)
          SET_REG_VALUE4(IDC_S2, m_s2)
          SET_REG_VALUE4(IDC_S3, m_s3)
          SET_REG_VALUE4(IDC_S4, m_s4)
          SET_REG_VALUE4(IDC_S5, m_s5)
          SET_REG_VALUE4(IDC_S6, m_s6)
          SET_REG_VALUE4(IDC_S7, m_s7)
          SET_REG_VALUE4(IDC_T8, m_t8)
          SET_REG_VALUE4(IDC_T9, m_t9)
          SET_REG_VALUE4(IDC_K0, m_k0)
          SET_REG_VALUE4(IDC_K1, m_k1)
          SET_REG_VALUE4(IDC_GP, m_gp)
          SET_REG_VALUE4(IDC_SP, m_sp)
          SET_REG_VALUE4(IDC_FP, m_fp)
          SET_REG_VALUE4(IDC_RA, m_ra)
          SET_REG_VALUE4(IDC_Lo, m_lo)
          SET_REG_VALUE4(IDC_Hi, m_hi)
          case IDC_SA:  g_cpu->m_sa = value; break;
          case IDC_PC:  g_cpu->m_pc = value; break;
        }
#undef SET_REG_VALUE4

        InvalidateRect(regwin->m_gpr_page->getHwnd(), NULL, 1);
        UpdateWindow(regwin->m_gpr_page->getHwnd());
      }
    }
    break;
  }
  return FALSE;
}

LRESULT CALLBACK RegisterWindow::COP0Proc(
                   HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp) {
  ds_dialog* dlg = (ds_dialog*)GetWindowLong(p_hwnd, DWL_USER);

  RegisterWindow* regwin = dlg ? (RegisterWindow*)dlg->getUserData() : NULL;

  switch (p_msg) {
  case WM_INITDIALOG:
    //SetWindowLong(GetDlgItem(hWnd, IDC_FP0), GWL_EXSTYLE, WS_EX_TRANSPARENT);
    return TRUE;
  case WM_CTLCOLOREDIT:

#define CHANGE_HIGHLIGHT(_id, _var)                               \
  case _id:                                                       \
    if (regwin->m_save_r5900.m_cop0._var != g_cpu->m_cop0._var) { \
      SetBkMode((HDC)p_wp, TRANSPARENT);                          \
      return (LRESULT)CreateSolidBrush(COLOR_READ);               \
    }                                                             \
    break;

    switch (regwin->hwnd2idc((HWND)p_lp, regwin->m_cop0_page->getHwnd())) {
      CHANGE_HIGHLIGHT(IDC_INDEX, index)
      CHANGE_HIGHLIGHT(IDC_RANDOM, random)
      CHANGE_HIGHLIGHT(IDC_ENTRYLO0, entrylo0)
      CHANGE_HIGHLIGHT(IDC_ENTRYLO1, entrylo1)
      CHANGE_HIGHLIGHT(IDC_CONTEXT, context)
      CHANGE_HIGHLIGHT(IDC_PAGEMASK, pagemask)
      CHANGE_HIGHLIGHT(IDC_WIRED, wired)
      CHANGE_HIGHLIGHT(IDC_RESERVED7, _reserved7)
      CHANGE_HIGHLIGHT(IDC_BADVADDR, badvaddr)
      CHANGE_HIGHLIGHT(IDC_COUNT, count)
      CHANGE_HIGHLIGHT(IDC_ENTERHI, entryhi)
      CHANGE_HIGHLIGHT(IDC_COMPARE, compare)
      CHANGE_HIGHLIGHT(IDC_STATUS, status)
      CHANGE_HIGHLIGHT(IDC_CAUSE, cause)
      CHANGE_HIGHLIGHT(IDC_EPC, epc)
      CHANGE_HIGHLIGHT(IDC_PRID, prid)
      CHANGE_HIGHLIGHT(IDC_CONFIG, config)
      CHANGE_HIGHLIGHT(IDC_RESERVED17, _reserved17)
      CHANGE_HIGHLIGHT(IDC_RESERVED18, _reserved18)
      CHANGE_HIGHLIGHT(IDC_RESERVED19, _reserved19)
      CHANGE_HIGHLIGHT(IDC_RESERVED21, _reserved20)
      CHANGE_HIGHLIGHT(IDC_RESERVED22, _reserved21)
      CHANGE_HIGHLIGHT(IDC_BADPADDR, badpaddr)
      CHANGE_HIGHLIGHT(IDC_BPC, bpc)
      CHANGE_HIGHLIGHT(IDC_PCCR, pccr)
      CHANGE_HIGHLIGHT(IDC_RESERVED26, _reserved26)
      CHANGE_HIGHLIGHT(IDC_RESERVED27, _reserved27)
      CHANGE_HIGHLIGHT(IDC_TAGLO, taglo)
      CHANGE_HIGHLIGHT(IDC_TAGHI, taghi)
      CHANGE_HIGHLIGHT(IDC_ERROREPC, errorepc)
      CHANGE_HIGHLIGHT(IDC_RESERVED31, _reserved31)
    }

    #undef CHANGE_HIGHLIGHT
    return NULL_BRUSH;
  case WM_COMMAND:
    if (regwin->m_ignore_wm_command == false) {
      int id    = p_wp & 0xffff;
      int notify  = (p_wp >> 16) & 0xffff;
      int hwnd  = p_lp;
      if (notify == EN_CHANGE) {
        char  str[256];
        GetDlgItemText(p_hwnd, id, str, 256);
        u32 value = ds_util::atox(str, 16);

        if (regwin->m_user_modified == false)
        {
          // 命令実行後のユーザによる初回の値変更のタイミングでCPUの状態をコピー
          memcpy(&regwin->m_save_r5900, g_cpu, sizeof(R5900REGS));
          regwin->m_user_modified = true;
        }
        
        regwin->m_ignore_wm_command = true;
        switch (id) {
        case IDC_INDEX:      g_cpu->m_cop0.index = value; break;
        case IDC_RANDOM:     g_cpu->m_cop0.random = value; break;
        case IDC_ENTRYLO0:   g_cpu->m_cop0.entrylo0 = value; break;
        case IDC_ENTRYLO1:   g_cpu->m_cop0.entrylo1 = value; break;
        case IDC_CONTEXT:    g_cpu->m_cop0.context = value; break;
        case IDC_PAGEMASK:   g_cpu->m_cop0.pagemask = value; break;
        case IDC_WIRED:      g_cpu->m_cop0.wired = value; break;
        case IDC_RESERVED7:  g_cpu->m_cop0._reserved7 = value; break;
        case IDC_BADVADDR:   g_cpu->m_cop0.badvaddr = value; break;
        case IDC_COUNT:      g_cpu->m_cop0.count = value; break;
        case IDC_ENTERHI:    g_cpu->m_cop0.entryhi = value; break;
        case IDC_COMPARE:    g_cpu->m_cop0.compare = value; break;
        case IDC_STATUS:     g_cpu->m_cop0.status = value; break;
        case IDC_CAUSE:      g_cpu->m_cop0.cause = value; break;
        case IDC_EPC:        g_cpu->m_cop0.epc = value; break;
        case IDC_PRID:       g_cpu->m_cop0.prid = value; break;
        case IDC_CONFIG:     g_cpu->m_cop0.config = value; break;
        case IDC_RESERVED17: g_cpu->m_cop0._reserved17 = value; break;
        case IDC_RESERVED18: g_cpu->m_cop0._reserved18 = value; break;
        case IDC_RESERVED19: g_cpu->m_cop0._reserved19 = value; break;
        case IDC_RESERVED21: g_cpu->m_cop0._reserved20 = value; break;
        case IDC_RESERVED22: g_cpu->m_cop0._reserved21 = value; break;
        case IDC_BADPADDR:   g_cpu->m_cop0.badpaddr = value; break;
        case IDC_BPC:        g_cpu->m_cop0.bpc = value; break;
        case IDC_PCCR:       g_cpu->m_cop0.pccr = value; break;
        case IDC_RESERVED26: g_cpu->m_cop0._reserved26 = value; break;
        case IDC_RESERVED27: g_cpu->m_cop0._reserved27 = value; break;
        case IDC_TAGLO:      g_cpu->m_cop0.taglo = value; break;
        case IDC_TAGHI:      g_cpu->m_cop0.taghi = value; break;
        case IDC_ERROREPC:   g_cpu->m_cop0.errorepc = value; break;
        case IDC_RESERVED31: g_cpu->m_cop0._reserved31 = value; break;
        }
        regwin->m_ignore_wm_command = false;

        InvalidateRect(regwin->m_cop0_page->getHwnd(), NULL, 1);
        UpdateWindow(regwin->m_cop0_page->getHwnd());
      }
    }
    break;
  }
  return FALSE;
}

LRESULT CALLBACK RegisterWindow::FPRProc(HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp) {

  ds_dialog* dlg = (ds_dialog*)GetWindowLong(p_hwnd, DWL_USER);

  RegisterWindow* regwin = dlg ? (RegisterWindow*)dlg->getUserData() : NULL;

  switch (p_msg) {
    case WM_INITDIALOG:
      //SetWindowLong(GetDlgItem(hWnd, IDC_FP0), GWL_EXSTYLE, WS_EX_TRANSPARENT);
      return TRUE;
    case WM_CTLCOLOREDIT:

#define CHANGE_HIGHLIGHT(_idx)                                            \
  case IDC_FPR##_idx:                                                     \
  case IDC_FPR_F##_idx:                                                   \
    if (regwin->m_save_r5900.m_fpu.fpr[_idx] != g_cpu->m_fpu.fpr[_idx]) { \
      SetBkMode((HDC)p_wp, TRANSPARENT);                                  \
      return (LRESULT)CreateSolidBrush(COLOR_READ);                       \
    }                                                                     \
    break;

      switch (regwin->hwnd2idc((HWND)p_lp, regwin->m_fpr_page->getHwnd())) {
      CHANGE_HIGHLIGHT(0)
      CHANGE_HIGHLIGHT(1)
      CHANGE_HIGHLIGHT(2)
      CHANGE_HIGHLIGHT(3)
      CHANGE_HIGHLIGHT(4)
      CHANGE_HIGHLIGHT(5)
      CHANGE_HIGHLIGHT(6)
      CHANGE_HIGHLIGHT(7)
      CHANGE_HIGHLIGHT(8)
      CHANGE_HIGHLIGHT(9)
      CHANGE_HIGHLIGHT(10)
      CHANGE_HIGHLIGHT(11)
      CHANGE_HIGHLIGHT(12)
      CHANGE_HIGHLIGHT(13)
      CHANGE_HIGHLIGHT(14)
      CHANGE_HIGHLIGHT(15)
      CHANGE_HIGHLIGHT(16)
      CHANGE_HIGHLIGHT(17)
      CHANGE_HIGHLIGHT(18)
      CHANGE_HIGHLIGHT(19)
      CHANGE_HIGHLIGHT(20)
      CHANGE_HIGHLIGHT(21)
      CHANGE_HIGHLIGHT(22)
      CHANGE_HIGHLIGHT(23)
      CHANGE_HIGHLIGHT(24)
      CHANGE_HIGHLIGHT(25)
      CHANGE_HIGHLIGHT(26)
      CHANGE_HIGHLIGHT(27)
      CHANGE_HIGHLIGHT(28)
      CHANGE_HIGHLIGHT(29)
      CHANGE_HIGHLIGHT(30)
      CHANGE_HIGHLIGHT(31)
      case IDC_ACC:
      case IDC_ACC_F:
        if (regwin->m_save_r5900.m_fpu.acc != g_cpu->m_fpu.acc) {
          SetBkMode((HDC)p_wp, TRANSPARENT);
          return (LRESULT)CreateSolidBrush(COLOR_READ);
        }
        break;
      case IDC_FCR0:
        if (regwin->m_save_r5900.m_fpu.fcr0.d != g_cpu->m_fpu.fcr0.d){
          SetBkMode((HDC)p_wp, TRANSPARENT);
          return (LRESULT)CreateSolidBrush(COLOR_READ);
        }
        break;
      case IDC_FCR31:
        if (regwin->m_save_r5900.m_fpu.fcr31 != g_cpu->m_fpu.fcr31){
          SetBkMode((HDC)p_wp, TRANSPARENT);
          return (LRESULT)CreateSolidBrush(COLOR_READ);
        }
        break;
      }
  #undef CHANGE_HIGHLIGHT

      return NULL_BRUSH;
    case WM_COMMAND:
      if (regwin->m_ignore_wm_command == false) {
        int id     = p_wp & 0xffff;
        int notify = (p_wp >> 16) & 0xffff;
        int hwnd   = p_lp;
        if (notify == EN_CHANGE) {
          char  str[256];
          GetDlgItemText(p_hwnd, id, str, 256);
          
#define SET_REG_VALUE(_idx)                       \
  case IDC_FPR##_idx: {                           \
    u32 value = ds_util::atox(str, 16);           \
    *((u32*)&g_cpu->m_fpu.fpr[_idx]) = value;     \
    sprintf(str, "%f", *((float*)&value));        \
    SetDlgItemText(p_hwnd, IDC_FPR_F##_idx, str); \
  } break;                                        \
  case IDC_FPR_F##_idx: {                         \
    float value = (float)atof(str);               \
    g_cpu->m_fpu.fpr[_idx] = value;               \
    sprintf(str, "%08x", *((u32*)&value));        \
    SetDlgItemText(p_hwnd, IDC_FPR##_idx, str);   \
    break; }

          if (regwin->m_user_modified == false) {
            // 命令実行後のユーザによる初回の値変更のタイミングでCPUの状態をコピー
            memcpy(&regwin->m_save_r5900, g_cpu, sizeof(R5900REGS));
            regwin->m_user_modified = true;
          }
          
          regwin->m_ignore_wm_command = true;
          switch (id) {
            SET_REG_VALUE(0)
            SET_REG_VALUE(1)
            SET_REG_VALUE(2)
            SET_REG_VALUE(3)
            SET_REG_VALUE(4)
            SET_REG_VALUE(5)
            SET_REG_VALUE(6)
            SET_REG_VALUE(7)
            SET_REG_VALUE(8)
            SET_REG_VALUE(9)
            SET_REG_VALUE(10)
            SET_REG_VALUE(11)
            SET_REG_VALUE(12)
            SET_REG_VALUE(13)
            SET_REG_VALUE(14)
            SET_REG_VALUE(15)
            SET_REG_VALUE(16)
            SET_REG_VALUE(17)
            SET_REG_VALUE(18)
            SET_REG_VALUE(19)
            SET_REG_VALUE(20)
            SET_REG_VALUE(21)
            SET_REG_VALUE(22)
            SET_REG_VALUE(23)
            SET_REG_VALUE(24)
            SET_REG_VALUE(25)
            SET_REG_VALUE(26)
            SET_REG_VALUE(27)
            SET_REG_VALUE(28)
            SET_REG_VALUE(29)
            SET_REG_VALUE(30)
            SET_REG_VALUE(31)
            case IDC_ACC: {
              u32 value = ds_util::atox(str, 16);
              *((u32*)&g_cpu->m_fpu.acc) = value;
              sprintf(str, "%f", *((float*)&value));
              SetDlgItemText(p_hwnd, IDC_ACC_F, str);
            } break;
            case IDC_ACC_F: {
              float value = (float)atof(str);
              g_cpu->m_fpu.acc = value;
              sprintf(str, "%08x", *((u32*)&value));
              SetDlgItemText(p_hwnd, IDC_ACC, str);
            } break;
            case IDC_FCR0: {
              u32 value = ds_util::atox(str, 16);
              *((u32*)&g_cpu->m_fpu.fcr0) = value;
            } break;
            case IDC_FCR31: {
              u32 value = ds_util::atox(str, 16);
              *((u32*)&g_cpu->m_fpu.fcr31) = value;
            } break;
          }
          regwin->m_ignore_wm_command = false;

          InvalidateRect(regwin->m_fpr_page->getHwnd(), NULL, 1);
          UpdateWindow(regwin->m_fpr_page->getHwnd());

#undef SET_REG_VALUE
        }
      }
      break;
  }
  return FALSE;
}
