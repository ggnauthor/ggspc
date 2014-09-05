/*---------*/
/* include */
/*---------*/
#include "main.h"
#include "debugger/debugger.h"
#include "debugger/gui/dialog/memory_edit_dialog.h"
#include "debugger/gui/window/memory_window.h"

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

/*---------*/
/* structs */
/*---------*/

/*--------*/
/* global */
/*--------*/
int g_edit_row = -1;
int g_edit_col = -1;
MemLabel* g_mem_label = NULL;

/*----------*/
/* function */
/*----------*/
int GetTypeIdx(char* p_type)
{
  for (int i = 0; Debugger::TYPE_DEF[i].size >= 0; i++) {
    if (strcmp(Debugger::TYPE_DEF[i].type_str, p_type) == 0) {
      return i;
    }
  }
  return -1;
}

int GetTypeSize(char* p_type)
{
  int idx = GetTypeIdx(p_type);
  if (idx != -1) {
    return Debugger::TYPE_DEF[idx].size;  
  } else {
    return 0;
  }
}

void InsertListColumn(HWND p_list, int p_col, char* p_label, int p_width, int p_align)
{
  LV_COLUMN  lvc;
  memset(&lvc, 0, sizeof(LV_COLUMN));
  lvc.mask    = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
  lvc.iSubItem  = p_col;
  lvc.pszText    = p_label;
  lvc.fmt      = p_align;
  lvc.cx      = p_width;
  ListView_InsertColumn(p_list, lvc.iSubItem, &lvc);
}

void InsertListItem(HWND p_list, int p_idx, int p_col, const char* p_label)
{
  LV_ITEM lvi;
  memset(&lvi, 0, sizeof(LV_ITEM));
  lvi.mask    = LVIF_TEXT;
  lvi.pszText    = const_cast<LPSTR>(p_label);
  lvi.iItem    = p_idx;
  lvi.iSubItem  = p_col;
  ListView_InsertItem(p_list, &lvi);
}

void SetListItem(HWND p_list, int p_idx, int p_col, char* p_label)
{
  LV_ITEM lvi;
  memset(&lvi, 0, sizeof(LV_ITEM));
  lvi.mask    = LVIF_TEXT;
  lvi.pszText    = p_label;
  lvi.iItem    = p_idx;
  lvi.iSubItem  = p_col;
  ListView_SetItem(p_list, &lvi);
}

void DeleteSelectListItem(HWND p_list)
{
  while(1) {
    int idx = ListView_GetNextItem(p_list, -1, LVNI_ALL | LVNI_SELECTED);
    if (idx == -1) break;
    ListView_DeleteItem(p_list, idx);
  } 
}

void UpdateStructureSize(HWND p_hdlg)
{
  // type�ɏ]���Ċe�����o�̃T�C�Y���X�V���A���v�T�C�Y���Z�o����
  HWND lst = GetDlgItem(p_hdlg, IDC_LST);
  int item_count = ListView_GetItemCount(lst);
  int total_size = 0;
  for (int i = 0; i < item_count; i++) {
    char str[256];
    ListView_GetItemText(lst, i, 0, str, 255);
    int size = GetTypeSize(str);
    if (size > 0) {
      _itoa(size, str, 10);
      ListView_SetItemText(lst, i, 1, str);
    } else {
      // �σT�C�Y�Ȃ�type�ɂ��X�V�͂��Ȃ�
      ListView_GetItemText(lst, i, 1, str, 255);
    }
    total_size += atoi(str);
  }

  // �\���̃T�C�Y
  char str[256];
  _itoa(total_size, str, 10);
  SetDlgItemText(p_hdlg, IDC_EDT_STRSIZE, str);
}

void OnDoubleClickListView(HWND p_hdlg, NMLISTVIEW* p_nmlv)
{
  HWND lst = GetDlgItem(p_hdlg, IDC_LST);
  HWND edt = GetDlgItem(p_hdlg, IDC_EDT_ELM_TEXT);
  HWND cmb = GetDlgItem(p_hdlg, IDC_CMB_ELM_TYPE);

  LVHITTESTINFO lvhti;
  memset(&lvhti, 0, sizeof(lvhti));
  lvhti.pt = p_nmlv->ptAction;
  ListView_SubItemHitTest(lst, &lvhti);
  if (lvhti.flags & LVHT_ONITEM) {
    g_edit_row = lvhti.iItem;
    g_edit_col = lvhti.iSubItem;

    // �ҏW�Z���̋�`���擾
    RECT rect;
    ListView_GetSubItemRect(lst, g_edit_row, g_edit_col, LVIR_LABEL, &rect);  // �Ȃ���LVIR_BOUNDS����0�J�����̕������������Ȃ�
    // �ʒu�ƃT�C�Y�𒲐�
    rect.left  += 4;
    rect.right  += 0;
    rect.top  -= 1;
    rect.bottom  += 2;
    MapWindowPoints(lst, p_hdlg, (POINT*)&rect, 2);

    char str[1024];
    ListView_GetItemText(lst, g_edit_row, g_edit_col, str, 1023);

    if (lvhti.iSubItem == 0) {
      // �R���{�{�b�N�X��ҏW�ΏۃZ����Ɉړ�
      rect.left -= 6;
      rect.top -= 3;
      rect.bottom -= 3;
      MoveWindow(cmb, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
      SendMessage(cmb, CB_SELECTSTRING, 0L, (LPARAM)str);
      ShowWindow(cmb, SW_SHOW);

      SetFocus(cmb);
    } else if (lvhti.iSubItem == 1) {
      // �܂��̓T�C�Y���ς��ǂ����`�F�b�N����
      char type[256];
      ListView_GetItemText(lst, g_edit_row, 0, type, 255);
      if (GetTypeSize(type) != 0) return;
      
      // ���l���́E�E�l�߂ɂ���
      long style = GetWindowLong(edt, GWL_STYLE);
      style |= ES_NUMBER | ES_RIGHT;
      OffsetRect(&rect, -4, 0);
      SetWindowLong(edt, GWL_STYLE, style);

      // �G�f�B�b�g�{�b�N�X��ҏW�ΏۃZ����Ɉړ�
      MoveWindow(edt, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
      SetWindowText(edt, str);
      ShowWindow(edt, SW_SHOW);

      // �e�L�X�g��S�I����Ԃɂ��ăt�H�[�J�X����
      SendMessage(edt, EM_SETSEL, 0, -1);
      SetFocus(edt);
    } else if (lvhti.iSubItem == 2) {
      // �������́E���l�߂ɂ���
      long style = GetWindowLong(edt, GWL_STYLE);
      style &= ~(ES_NUMBER | ES_RIGHT);
      SetWindowLong(edt, GWL_STYLE, style);

      // �G�f�B�b�g�{�b�N�X��ҏW�ΏۃZ����Ɉړ�
      MoveWindow(edt, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
      SetWindowText(edt, str);
      ShowWindow(edt, SW_SHOW);

      // �e�L�X�g��S�I����Ԃɂ��ăt�H�[�J�X����
      SendMessage(edt, EM_SETSEL, 0, -1);
      SetFocus(edt);
    }
  }
}

LRESULT CALLBACK HookListViewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == WM_LBUTTONDOWN) {
    HWND dlg = GetParent(hWnd);
    HWND edt = GetDlgItem(dlg, IDC_EDT_ELM_TEXT);
    HWND cmb = GetDlgItem(dlg, IDC_CMB_ELM_TYPE);

    if (IsWindowVisible(edt)) {
      POINT pt = { LOWORD(lParam), HIWORD(lParam) };
      ClientToScreen(hWnd, &pt);
      RECT rect;
      GetWindowRect(edt, &rect);
      if (PtInRect(&rect, pt)) {
        ScreenToClient(edt, &pt);
        SendMessage(edt, WM_LBUTTONDOWN, 0L, MAKELPARAM(pt.x, pt.y));
        return TRUE;
      }
    } else if (IsWindowVisible(cmb)) {
      POINT pt = { LOWORD(lParam), HIWORD(lParam) };
      ClientToScreen(hWnd, &pt);
      RECT rect;
      GetWindowRect(cmb, &rect);
      if (PtInRect(&rect, pt)) {
        ScreenToClient(cmb, &pt);
        SendMessage(cmb, WM_LBUTTONDOWN, 0L, MAKELPARAM(pt.x, pt.y));
        return TRUE;
      }
    }
  }
  
  WNDPROC default_proc = (WNDPROC)GetWindowLong(hWnd, GWL_USERDATA);
  return CallWindowProc(default_proc, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK HookListPartWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == WM_KILLFOCUS) {
    HWND dlg = GetParent(hWnd);
    HWND lst = GetDlgItem(dlg, IDC_LST);

    if (g_edit_row >= 0) {
      char str[1024];
      GetWindowText(hWnd, str, 1023);

      char oldstr[1024];
      ListView_GetItemText(lst, g_edit_row, g_edit_col, oldstr, 1023);
      if (str != oldstr) {  // �ύX����Ă����烊�X�g�ɔ��f
        // �ҏW�ΏۃZ���ɁC�G�f�B�b�g�{�b�N�X�w����e�𔽉f
        ListView_SetItemText(lst, g_edit_row, g_edit_col, str);
        UpdateStructureSize(dlg);
      }
      ShowWindow(hWnd, SW_HIDE);
    }
  }
  WNDPROC default_proc = (WNDPROC)GetWindowLong(hWnd, GWL_USERDATA);
  return CallWindowProc(default_proc, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK MemEditDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  u32  va = g_dbg->memory_window()->select();

  switch (message) {
  case WM_INITDIALOG: {
    // ���X�g�r���[
    HWND lst = GetDlgItem(hDlg, IDC_LST);
    g_dbg->setFontToFixed(lst);
    // �T�u�N���X��
    SetWindowLong(lst, GWL_USERDATA, GetWindowLong(lst, GWL_WNDPROC));
    SetWindowLong(lst, GWL_WNDPROC, (LONG)HookListViewProc);
    // �s�P�ʂőI��
    DWORD style = ListView_GetExtendedListViewStyle(lst);
    ListView_SetExtendedListViewStyle(lst, style | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    // �J�����̐ݒ�
    InsertListColumn(lst, 0, "type", 80, LVCFMT_LEFT);
    InsertListColumn(lst, 1, "size", 48, LVCFMT_RIGHT);
    InsertListColumn(lst, 2, "label", 210, LVCFMT_LEFT);

    // ���x��
    MemLabel* mem_label = NULL;
    int idx;
    int offset = g_dbg->getMemLabelIdx(va, &idx);
    if (offset != -1) {
      mem_label = g_dbg->getMemLabel(idx);

      // �������E�C���h�E��̃t�H�[�J�X�����x���̐擪���w���Ă���Ƃ͌���Ȃ��̂ŁA�C��
      va = mem_label->va();

      SetDlgItemText(hDlg, IDC_EDT_LABEL, mem_label->text());

      SetDlgItemText(hDlg, IDC_EDT_COMMENT, mem_label->comment());

      char count_str[256];
      _itoa(mem_label->count(), count_str, 10);
      SetDlgItemText(hDlg, IDC_EDT_COUNT, count_str);

      for (int i = 0; i < mem_label->element_size(); i++) {
        InsertListItem(lst, i, 0, Debugger::TYPE_DEF[mem_label->element(i)->type].type_str);

        char size_str[256];
        _itoa(mem_label->element(i)->size, size_str, 10);
        SetListItem(lst, i, 1, size_str);
        SetListItem(lst, i, 2, mem_label->element(i)->label);
      }
      UpdateStructureSize(hDlg);  // �\���̃T�C�Y
    } else {
      SetDlgItemText(hDlg, IDC_EDT_LABEL, "");
      SetDlgItemText(hDlg, IDC_EDT_COUNT, "1");

      HWND lst = GetDlgItem(hDlg, IDC_LST);
      InsertListItem(lst, 0, 0, "hex");
      SetListItem(lst, 0, 1, "4");
      SetListItem(lst, 0, 2, "");

      SetDlgItemText(hDlg, IDC_EDT_STRSIZE, "4");
    }

    // �A�h���X
    char str[1024];
    sprintf(str, "%08x", va);
    SetDlgItemText(hDlg, IDC_EDT_ADDR, str);
    g_dbg->setFontToFixed(GetDlgItem(hDlg, IDC_EDT_ADDR));

    // ���X�g���̓G�f�B�b�g�{�b�N�X
    HWND edt = GetDlgItem(hDlg, IDC_EDT_ELM_TEXT);
    g_dbg->setFontToFixed(edt);
    // �T�u�N���X��
    SetWindowLong(edt, GWL_USERDATA, GetWindowLong(edt, GWL_WNDPROC));
    SetWindowLong(edt, GWL_WNDPROC, (LONG)HookListPartWndProc);
    ShowWindow(edt, SW_HIDE);

    // ���X�g����type�R���{�{�b�N�X
    // ���̃^�C�~���O�ł�MemoryWindow�̏��������������Ă��Ȃ��Bm_dlg���g���Ȃ��̂Œ��ӁB
    HWND cmb = GetDlgItem(hDlg, IDC_CMB_ELM_TYPE);
    g_dbg->setFontToFixed(cmb);
    // �T�u�N���X��
    SetWindowLong(cmb, GWL_USERDATA, GetWindowLong(cmb, GWL_WNDPROC));
    SetWindowLong(cmb, GWL_WNDPROC, (LONG)HookListPartWndProc);
    ShowWindow(cmb, SW_HIDE);
    // ���ݒ�
    for (int i = 0; Debugger::TYPE_DEF[i].size >= 0; i++) {
      SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)Debugger::TYPE_DEF[i].type_str);
    }
    SendMessage(cmb, CB_SETCURSEL, 0L, 0L);

    HWND chk_detail = GetDlgItem(hDlg, IDC_CHK_DETAIL);
    SendMessage(chk_detail, BM_SETCHECK, BST_UNCHECKED, 0);

    RECT rect;
    GetWindowRect(hDlg, &rect);
    SetWindowPos(hDlg, NULL, rect.left, rect.top - (rect.bottom - rect.top + 20), 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    SetFocus(GetDlgItem(hDlg, IDC_EDT_LABEL));
    return FALSE; }
  case WM_DESTROY:
    break;
  case WM_COMMAND:
    {
      switch (LOWORD(wParam)) {
      case IDOK: {
        // �`�F�b�N
        HWND lst = GetDlgItem(hDlg, IDC_LST);
        int item_count = ListView_GetItemCount(lst);
        if (item_count >= 2) {
          for (int i = 0; i < item_count; i++) {
            char str[256];
            ListView_GetItemText(lst, i, 2, str, 255);
            ds_util::strtrim(str);
            if (str[0] == '\0') {  // �����ȃ��x��
              MessageBox(hDlg, "�����o����������ꍇ�A���ׂẴ����o�Ƀ��x�������K�v�ł�", NULL, NULL);
              return FALSE;
            }
          }
        }
        // ���x���𔽉f
        char label_str[256];
        GetDlgItemText(hDlg, IDC_EDT_LABEL, label_str, 255);
        label_str[255] = '\0';
        ds_util::strtrim(label_str);

        // �R�����g�𔽉f
        char comment_str[256];
        GetDlgItemText(hDlg, IDC_EDT_COMMENT, comment_str, 255);
        comment_str[255] = '\0';
        ds_util::strtrim(comment_str);

        // �J�E���g�𔽉f
        char count_str[256];
        GetDlgItemText(hDlg, IDC_EDT_COUNT, count_str, 255);
        count_str[255] = '\0';
        int count = atoi(count_str);
        if (count < 1) count = 1;

        MemLabel* label = NULL;
        int idx;
        if (g_dbg->getMemLabelIdx(va, &idx) == -1) {
          if (label_str[0] != '\0') {
            // �V�K�ǉ��E�����čX�V������
            label = new MemLabel(g_dbg->get_current_ddb(), va, label_str);
            g_dbg->addMemLabel(label);
          } else {
            // �Ȃɂ����Ȃ�
            EndDialog(hDlg, IDOK);
            break;
          }
        } else {
          label = g_dbg->getMemLabel(idx);
        }
        assert(label);
        if (label_str[0] != '\0') {
          // �X�V
          label->set_text(label_str);
          label->set_comment(comment_str);
          label->set_count(count);

          label->clearElement();
          HWND lst = GetDlgItem(hDlg, IDC_LST);
          int item_count = ListView_GetItemCount(lst);
          for (int j = 0; j < item_count; j++) {
            char str[256];
            int type;
            int size;

            ListView_GetItemText(lst, j, 0, str, 255);
            type = GetTypeIdx(str);
            ListView_GetItemText(lst, j, 1, str, 255);
            size = atoi(str);
            ListView_GetItemText(lst, j, 2, str, 255);
            label->AddElement(type, size, str);
          }
        } else {
          // �폜
          int idx;
          g_dbg->getMemLabelIdx(label->va(), &idx);
          g_dbg->deleteMemLabel(idx);
        }
        EndDialog(hDlg, IDOK);
        break; }
      case IDCANCEL:
        EndDialog(hDlg, IDCANCEL);
        break;
      case IDC_BTN_ADD: {
        HWND lst = GetDlgItem(hDlg, IDC_LST);
        int selidx = ListView_GetNextItem(lst, -1, LVNI_ALL | LVNI_SELECTED);
        if (selidx == -1) {
          // �I������ĂȂ���Ζ����ɒǉ�����
          selidx = ListView_GetItemCount(lst);
        }
        InsertListItem(lst, selidx, 0, "hex");
        SetListItem(lst, selidx, 1, "4");
        SetListItem(lst, selidx, 2, "");

        UpdateStructureSize(hDlg);
        break; }
      case IDC_BTN_DEL:
        // �����o�͂P�ȏ�Ȃ��Ă͂Ȃ�Ȃ�
        if (ListView_GetItemCount(GetDlgItem(hDlg, IDC_LST)) > 1) {
          DeleteSelectListItem(GetDlgItem(hDlg, IDC_LST));
          UpdateStructureSize(hDlg);
        } else {
          MessageBox(hDlg, "�����o�͂P�ȏ�K�v�ł�", NULL, NULL);
        }
        break;
      default:
        return FALSE;
      }
    }
    return FALSE;
  case WM_NOTIFY: {
    NMHDR* nmhdr = reinterpret_cast<NMHDR*>(lParam);
    if (nmhdr->hwndFrom == GetDlgItem(hDlg, IDC_LST)) {
      switch (nmhdr->code) {
      case NM_DBLCLK:  OnDoubleClickListView(hDlg, reinterpret_cast<NMLISTVIEW*>(nmhdr)); break;
      }
    }
    break; }
  default:
    return FALSE; 
  }
  return TRUE;
}
