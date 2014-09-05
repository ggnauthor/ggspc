/*---------------------------------------------------------------*/
/*                           ds_window.cpp                       */
/*---------------------------------------------------------------*/

#include "_prefix.h"

#pragma comment(lib, "comctl32.lib")

/*---------*/
/* include */
/*---------*/
#include "ds_window.h"

#include <commctrl.h>

/*--------*/
/* method */
/*--------*/

///////////////////////////////////////////////////////////////////////////
// win_base
LONG win_base::getStyle(bool p_ex)
{
	return GetWindowLong(m_hwnd, p_ex ? GWL_EXSTYLE : GWL_STYLE);
}

void win_base::setStyle(LONG p_style, bool p_ex)
{
	SetWindowLong(m_hwnd, p_ex ? GWL_EXSTYLE : GWL_STYLE, p_style);
}

void win_base::topmost(bool p_enable)
{
	SetWindowPos(m_hwnd, p_enable ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void win_base::size(int p_w, int p_h, bool p_adjust)
{
	RECT rect = { 0, 0, p_w, p_h };
	if (p_adjust) AdjustWindowRectEx(&rect, GetWindowLong(m_hwnd, GWL_STYLE), false, GetWindowLong(m_hwnd, GWL_EXSTYLE));
	SetWindowPos(m_hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
}

void win_base::move(int p_x, int p_y)
{
	RECT	rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	SetWindowPos(m_hwnd, NULL, p_x + rect.left, p_y + rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void win_base::move_center(void)
{
	//HDC hdc = GetDC(NULL);
	//int screenW = GetDeviceCaps(hdc, HORZRES);
	//int screenH = GetDeviceCaps(hdc, VERTRES); 
	//ReleaseDC(NULL, hdc);
	RECT	rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	SetWindowPos(m_hwnd, NULL,
		rect.left + (rect.right - rect.left - getWidth()) / 2,
		rect.top  + (rect.bottom - rect.top - getHeight()) / 2,
		0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void win_base::waitForCloseWindow(void)
{
	bool done = false;
	while (!done)
	{
		Sleep(10);

		tagMSG	msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT ||
				msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
			{
				done = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}

int win_base::getLeft(void)
{
	RECT rect;
	GetWindowRect(m_hwnd, &rect);
	return rect.left;
}

int win_base::getTop(void)
{
	RECT rect;
	GetWindowRect(m_hwnd, &rect);
	return rect.top;
}

int win_base::getWidth(void)
{
	RECT rect;
	GetClientRect(m_hwnd, &rect);
	return (rect.right - rect.left);
}

int win_base::getHeight(void)
{
	RECT rect;
	GetClientRect(m_hwnd, &rect);
	return (rect.bottom - rect.top);
}

RECT win_base::getRect(void)
{
	RECT rect;
	GetClientRect(m_hwnd, &rect);
	return rect;
}

void win_base::setCaption(char* p_caption)
{
	SetWindowTextA(m_hwnd, p_caption);
}

LRESULT CALLBACK defwndproc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

void win_base::createDC(void)
{
	if (m_hwnd == INVALID_HANDLE_VALUE) return;
	if (m_backDC != INVALID_HANDLE_VALUE) return;

	RECT rect;
	GetClientRect(m_hwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	m_bminfo = new BITMAPINFO;
	ZeroMemory(m_bminfo, sizeof(BITMAPINFO));
	m_bminfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bminfo->bmiHeader.biWidth = width;
	m_bminfo->bmiHeader.biHeight = height;
	m_bminfo->bmiHeader.biPlanes = 1;
	m_bminfo->bmiHeader.biBitCount = 32;
	m_bminfo->bmiHeader.biCompression = BI_RGB;

	m_bmp = CreateDIBSection(NULL, m_bminfo, DIB_RGB_COLORS, (void**)(&m_bmdata), NULL, 0);

	for (int i = 0; i < width * height; i++) { m_bmdata[i] = 0; }

	m_dc = GetDC(m_hwnd);
	m_backDC = CreateCompatibleDC(m_dc);
	m_bmp_old = (HBITMAP)SelectObject(m_backDC, (HGDIOBJ)m_bmp);
}

void win_base::releaseDC(void)
{
	if (m_backDC == INVALID_HANDLE_VALUE) return;

	delete m_bminfo;
	SelectObject(m_backDC, m_bmp_old);
	DeleteObject(m_bmp);
	DeleteDC(m_backDC);
	ReleaseDC(m_hwnd, m_dc);

	m_dc		= (HDC)INVALID_HANDLE_VALUE;
	m_bminfo	= NULL;
	m_bmp		= (HBITMAP)INVALID_HANDLE_VALUE;
	m_bmp_old	= (HBITMAP)INVALID_HANDLE_VALUE;
	m_bmdata	= NULL;
	m_backDC	= (HDC)INVALID_HANDLE_VALUE;
}

void win_base::update(void)
{
	update(0, 0, getWidth(), getHeight());
}

void win_base::update(int p_left, int p_top, int p_right, int p_bottom)
{
	BitBlt(m_dc, p_left, p_top, p_right - p_left, p_bottom - p_top, m_backDC, p_left, p_top, SRCCOPY);
}

BOOL CALLBACK _enum_child_proc(HWND hWnd, LPARAM lp)
{
	HWND* before = (HWND*)lp;
	if (hWnd == *before)
	{
		// 次に見つけたらそれを返す
		*before = NULL;
	}
	else if (*before == NULL)
	{
		// このウインドウを返す
		*before = hWnd;
		return FALSE;
	}
	return TRUE;
}

HWND win_base::findChildWindow(HWND p_wnd)
{
	EnumChildWindows(m_hwnd, _enum_child_proc, (LPARAM)&p_wnd);

	return p_wnd;
}

///////////////////////////////////////////////////////////////////////////
// ds_window
ds_window::ds_window(char* p_wc, WNDPROC p_wndproc, int p_width, int p_height, LONG p_style)
{
	m_hinst = GetModuleHandle(NULL);
	strcpy(m_wc, p_wc);
	
	WNDCLASSA	wc;
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= p_wndproc ? p_wndproc : defwndproc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= m_hinst;
	wc.hIcon			= LoadIcon(m_hinst, NULL);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName		= "";
	wc.lpszClassName	= m_wc;
	if (RegisterClassA(&wc) == 0) {
    //APPERR("Failed to regist windowclass.");
	}

	//HDC hdc = GetDC(NULL);
	//int screenW = GetDeviceCaps(hdc, HORZRES);
	//int screenH = GetDeviceCaps(hdc, VERTRES); 
	//ReleaseDC(NULL, hdc);
	RECT	rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	m_hwnd = CreateWindowA(m_wc, "", p_style,
		rect.left + (rect.right - rect.left - p_width) / 2,
		rect.top  + (rect.bottom - rect.top - p_height) / 2,
		p_width, p_height, NULL, NULL, m_hinst, NULL);
	ShowWindow(m_hwnd, SW_HIDE);
	SetWindowTextA(m_hwnd, m_wc);

	SetWindowLong(m_hwnd, GWL_USERDATA, (LONG)this);

	size(p_width, p_height);
}

ds_window::~ds_window(void)
{
	DestroyWindow(m_hwnd);
	if (UnregisterClassA(m_wc, m_hinst) == 0) {
    //printf("Failed to regist windowclass.");
  }
}

///////////////////////////////////////////////////////////////////////////
// ds_dialog
struct TabData
{
	int			tcid;
	int			tpdlgcount;
	ds_dialog*	tpdlg[64];
	bool		tpdlgmanaged[64];
};

LRESULT CALLBACK ds_dialog_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	ds_dialog* dlg = (ds_dialog*)GetWindowLong(hDlg, DWL_USER);
	assert(dlg);

	NMHDR *nm = (NMHDR*)lParam;

	switch (message)
	{
	case WM_NOTIFY:
		// タブコントロールの選択制御
		if (nm->code == TCN_SELCHANGE)
		{
			for (int i = 0; i < dlg->m_tabcount; i++)
			{
				HWND hwtc = GetDlgItem(dlg->m_hwnd, dlg->m_tabinfo[i]->tcid);
				if (nm->hwndFrom == hwtc)
				{
					int curtpid = TabCtrl_GetCurSel(hwtc);
					for (int j = 0; j < dlg->m_tabinfo[i]->tpdlgcount; j++)
					{
						if (j != curtpid)
						{
							ShowWindow(dlg->m_tabinfo[i]->tpdlg[j]->getHwnd(), SW_HIDE);
						}
					}
					ShowWindow(dlg->m_tabinfo[i]->tpdlg[curtpid]->getHwnd(), SW_SHOW);
				}
			}
		}
		break;
	}

	return dlg->m_userproc ? dlg->m_userproc(hDlg, message, wParam, lParam) : FALSE;
}

ds_dialog::ds_dialog(int p_resID, HWND p_parent, DLGPROC p_wnd_proc, LPARAM p_init_param)
{
	m_hinst = GetModuleHandle(NULL);

	// 作成時 (WM_INITDIALOG) は引数に指定したウインドウプロシージャのみが走り、
	// その後、プロシージャをds_dialog_procに差し替え、タブ制御が行われるようにする。
	m_userproc = p_wnd_proc;
	m_hwnd = CreateDialogParam(m_hinst, MAKEINTRESOURCE(p_resID), p_parent, (DLGPROC)p_wnd_proc, p_init_param);
	SetWindowLong(m_hwnd, DWL_USER, (LONG)this);
	SetWindowLong(m_hwnd, DWL_DLGPROC, (LONG)ds_dialog_proc);

	m_tabcount = 0;

	ShowWindow(m_hwnd, SW_HIDE);
	UpdateWindow(m_hwnd);
}

ds_dialog::~ds_dialog(void)
{
	for (int i = 0; i < m_tabcount; i++)
	{
		for (int j = 0; j < m_tabinfo[i]->tpdlgcount; j++)
		{
			if (m_tabinfo[i]->tpdlgmanaged[j]) delete m_tabinfo[i]->tpdlg[j];
		}
		delete m_tabinfo[i];
	}
	m_tabcount = 0;
}

bool ds_dialog::dispatchDialogMessage(MSG* p_msg, HWND p_filterCtl, char* p_filterVK)
{
	// キーダウンメッセージの場合…
	bool ignore_ctrl = false;
	if (p_msg->message == WM_KEYDOWN && p_msg->hwnd == p_filterCtl)
	{
		for (int i = 0; p_filterVK[i] != '\0'; i++)
		{
			// 入力した仮想キーが無視したいコードなら…
			if (p_filterVK[i] == p_msg->wParam)
			{
				HWND wnd = NULL;
				
				// すべてのダイアログ部品宛のメッセージであれば無視対象とする。
				do
				{
					wnd = findChildWindow(wnd);
					if (p_msg->hwnd == wnd)
					{
						ignore_ctrl = true;
						break;
					}
				}while (wnd);

				break;
			}
		}
	}

	if (ignore_ctrl)
	{
		// コントロール宛ての無視対象メッセージであった場合、
		// 親ダイアログに向けて同じメッセージを発行する
		SendMessage(m_hwnd, p_msg->message, p_msg->wParam, p_msg->lParam);
		return true;
	}
	else
	{
		// ダイアログ宛のコントロールメッセージを処理する
		if (IsDialogMessage(m_hwnd, p_msg) != 0)
		{
			// コントロールのキーダウンは親に伝わらないので意図的に送信する
			if (p_msg->message == WM_KEYDOWN)
			{
				// すべてのタブページについて同じメッセージを送る
				for (int i = 0; i < m_tabcount; i++)
				{
					int curtpid = TabCtrl_GetCurSel(GetDlgItem(m_hwnd, m_tabinfo[i]->tcid));
					SendMessage(m_tabinfo[i]->tpdlg[curtpid]->getHwnd(), p_msg->message, p_msg->wParam, p_msg->lParam);
				}
				// ダイアログ自身に同じメッセージを送る
				SendMessage(m_hwnd, p_msg->message, p_msg->wParam, p_msg->lParam);
			}
			return true;
		}
	}
	return false;
}

ds_dialog* ds_dialog::getCurrentPage(int p_tcid)
{
	for (int i = 0; i < m_tabcount; i++)
	{
		if (m_tabinfo[i]->tcid == p_tcid)
		{
			int curtpid = TabCtrl_GetCurSel(GetDlgItem(m_hwnd, p_tcid));
			return m_tabinfo[i]->tpdlg[curtpid];
		}
	}
	return NULL;
}

void ds_dialog::_insertTabPage(int p_tcid, ds_dialog* p_page, char* p_label, DLGPROC p_tpproc, bool p_managed)
{
	// 追加するタブ項目情報を設定
	TC_ITEM tcItem;
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = p_label;

	// タブコントロールの情報を取得
	HWND hwtc = GetDlgItem(m_hwnd, p_tcid);
	RECT tcrect;
	GetClientRect(hwtc, &tcrect);

	// タブコントロールが無ければ管理下に追加
	bool findtcid = false;
	for (int i = 0; i < m_tabcount; i++)
	{
		if (i == 16) throw EExcp_TabCtrlOver;
		if (m_tabinfo[i]->tcid == p_tcid)
		{
			if (m_tabinfo[i]->tpdlgcount == 64) throw EExcp_TabCtrlItemOver;
			m_tabinfo[i]->tpdlg[m_tabinfo[i]->tpdlgcount]			= p_page;
			m_tabinfo[i]->tpdlgmanaged[m_tabinfo[i]->tpdlgcount]	= p_managed;
			m_tabinfo[i]->tpdlgcount++;
			findtcid = true;
			// タブは後ろに追加していく
			SendMessage(hwtc, TCM_INSERTITEM, (WPARAM)m_tabinfo[i]->tpdlgcount, (LPARAM)&tcItem);
			// タブコントロールの表示域を取得し、ダイアログをマッピングする
			TabCtrl_AdjustRect(hwtc, false, &tcrect);
			MapWindowPoints(hwtc, m_hwnd, (POINT*)&tcrect, 2);	// left, rightを親ダイアログの座標系にマップする
			MoveWindow(p_page->getHwnd(), tcrect.left, tcrect.top, tcrect.right - tcrect.left, tcrect.bottom - tcrect.top, false);
			// ２つ目移行のタブは初期非表示
			ShowWindow(p_page->getHwnd(), SW_HIDE);
			break;
		}
	}
	if (!findtcid)	// 新規タブ
	{
		m_tabinfo[m_tabcount] = (struct TabData*)malloc(sizeof(struct TabData));
		m_tabinfo[m_tabcount]->tcid				= p_tcid;
		m_tabinfo[m_tabcount]->tpdlg[0]			= p_page;
		m_tabinfo[m_tabcount]->tpdlgmanaged[0]	= p_managed;
		m_tabinfo[m_tabcount]->tpdlgcount		= 1;
		m_tabcount++;
		// タブは先頭に追加
		SendMessage(hwtc, TCM_INSERTITEM, (WPARAM)0, (LPARAM)&tcItem);
		// タブコントロールの表示域を取得し、ダイアログをマッピングする
		TabCtrl_AdjustRect(hwtc, false, &tcrect);
		MapWindowPoints(hwtc, m_hwnd, (POINT*)&tcrect, 2);	// left, rightを親ダイアログの座標系にマップする
		MoveWindow(p_page->getHwnd(), tcrect.left, tcrect.top, tcrect.right - tcrect.left, tcrect.bottom - tcrect.top, false);
		// １つ目のタブは初期表示
		ShowWindow(p_page->getHwnd(), SW_SHOW);
	}
}

void ds_dialog::insertTabPage(int p_tcid, int p_tpid, char* p_label, DLGPROC p_tpproc)
{
	ds_dialog* page = new ds_dialog(p_tpid, m_hwnd, p_tpproc, NULL);

	_insertTabPage(p_tcid, page, p_label, p_tpproc, true);
}

void ds_dialog::insertTabPage(int p_tcid, ds_dialog* p_page, char* p_label)
{
	SetParent(m_hwnd, p_page->getHwnd());
	_insertTabPage(p_tcid, p_page, p_label, p_page->m_userproc, false);
}

///////////////////////////////////////////////////////////////////////////
// ds_filewindow
ds_filewindow::ds_filewindow(void)
{
	memset(&m_ofn, 0, sizeof(OPENFILENAME));
	m_ofn.lStructSize		= sizeof(OPENFILENAME);
	m_ofn.hwndOwner			= NULL;
	m_ofn.lpstrFile			= (LPSTR)malloc(MAX_PATH);
	m_ofn.nMaxFile			= MAX_PATH;
	m_ofn.lpstrFileTitle	= NULL;
	m_ofn.nMaxFileTitle		= NULL;
	m_ofn.nFilterIndex		= 1;
	m_ofn.lpstrFilter		= NULL;
	m_ofn.lpstrInitialDir	= NULL;

	m_ofn.lpstrFile[0] = '\0';
}

ds_filewindow::~ds_filewindow()
{
	free((void*)m_ofn.lpstrFile);
}

void ds_filewindow::setInitialDir(char* p_initdir)
{
	m_ofn.lpstrInitialDir = p_initdir;
}

bool ds_filewindow::openfile(const char* p_filter, DWORD p_flag)
{
	m_ofn.lpstrFilter	= p_filter ? p_filter : "All files(*.*)\0*.*\0\0";
	m_ofn.lpstrDefExt	= NULL;
	m_ofn.Flags			= p_flag;
	return !!GetOpenFileName(&m_ofn);
}

bool ds_filewindow::savefile(const char* p_filter, const char* p_def_file, const char* p_def_ext, DWORD p_flag)
{
	m_ofn.lpstrFilter	= p_filter ? p_filter : "All files(*.*)\0*.*\0\0";
	m_ofn.lpstrDefExt	= p_def_ext;
	m_ofn.Flags			= p_flag;
	strncpy(m_ofn.lpstrFile, p_def_file, MAX_PATH);
	return !!GetSaveFileName(&m_ofn);
}

#include "_postfix.h"
