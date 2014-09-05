/*---------------------------------------------------------------*/
/*                            ds_window.h                        */
/*---------------------------------------------------------------*/

#pragma once

/*---------*/
/* include */
/*---------*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*---------*/
/* define  */
/*---------*/
#define MODALDIALOG(_resid, _parent, _dlgproc)	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(_resid), _parent, (DLGPROC)_dlgproc)

/*-------*/
/* class */
/*-------*/
class win_base
{
protected:	/* win_baseのインスタンスは作成禁止 */
	win_base(void)
	{
		m_hinst		= (HINSTANCE)INVALID_HANDLE_VALUE;
		m_hwnd		= (HWND)INVALID_HANDLE_VALUE;
		m_active	= false;

		m_dc		= (HDC)INVALID_HANDLE_VALUE;
		m_bminfo	= NULL;
		m_bmp		= (HBITMAP)INVALID_HANDLE_VALUE;
		m_bmp_old	= (HBITMAP)INVALID_HANDLE_VALUE;
		m_bmdata	= NULL;
		m_backDC	= (HDC)INVALID_HANDLE_VALUE;
		m_userdata	= NULL;
	}
	~win_base(void)
	{
		releaseDC();
		if (m_hwnd) DestroyWindow(m_hwnd);
	}

public:
	void show(void) { ShowWindow(m_hwnd, SW_SHOW); UpdateWindow(m_hwnd); }
	void hide(void) { ShowWindow(m_hwnd, SW_HIDE); }
	
	bool fore(void) { return !!SetForegroundWindow(m_hwnd); }

	LONG getStyle(bool p_ex = false);
	void setStyle(LONG p_style, bool p_ex = false);
	void topmost(bool p_enable);
	void size(int p_w, int p_h, bool p_adjust = true);
	void move(int p_x, int p_y);
	void move_center(void);

	void waitForCloseWindow(void);
	
	HINSTANCE	getInstance(void)	{ return m_hinst; }
	HWND		getHwnd(void)		{ return m_hwnd; }
	HDC			getDC(void)			{ return m_dc; }
	HDC			getBackDC(void)		{ return m_backDC; }
	DWORD*		getBackBuffer(void)	{ return m_bmdata; }
	int			getLeft(void);
	int			getTop(void);

	int			getWidth(void);
	int			getHeight(void);
	RECT		getRect(void);

	void setActive(bool p_value){ m_active = p_value; }
	bool getActive(void)		{ return m_active; }

	// ds_winbaseではDWL_USERを使用してはならない！
	// 代わりに以下のメソッドにてユーザーデータを扱う
	void setUserData(DWORD p_data)	{ m_userdata = p_data; }
	DWORD getUserData(void)			{ return m_userdata; }

	void setCaption(char* p_caption);

	friend LRESULT CALLBACK defwndproc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	
	void createDC(void);
	void releaseDC(void);
	void update(void);
	void update(int p_left, int p_top, int p_right, int p_bottom);

	HWND findChildWindow(HWND p_wnd);

protected:
	HINSTANCE	m_hinst;
	HWND		m_hwnd;
	bool		m_active;
	
	DWORD		m_userdata;	// DWL_USERの代わり

	HDC					m_dc;
	BITMAPINFO*			m_bminfo;
	HBITMAP				m_bmp;
	HBITMAP				m_bmp_old;
	DWORD*				m_bmdata;
	HDC					m_backDC;
};

class ds_window : public win_base
{
public:
	ds_window(char* p_wc, WNDPROC p_wndproc, int p_width, int p_height, LONG p_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION);
	~ds_window(void);

protected:
	char		m_wc[256];
};

class ds_dialog : public win_base
{
public:
	enum EExcp
	{
		EExcp_TabCtrlOver = 0,	// ダイアログあたりのタブコントロール数が多すぎ
		EExcp_TabCtrlItemOver,	// タブコントロールあたりの項目数が多すぎ
	};

	ds_dialog(int p_resID, HWND p_parent, DLGPROC p_wndproc, LPARAM p_init_param);
	~ds_dialog(void);
	
	bool dispatchDialogMessage(MSG* p_msg, HWND p_filterCtl, char* p_filterVK = "");
	ds_dialog* getCurrentPage(int p_tcid);

	void insertTabPage(int p_tcid, int p_tpid, char* p_label, DLGPROC p_tpproc);
	void insertTabPage(int p_tcid, ds_dialog* p_page, char* p_label);

	friend LRESULT CALLBACK ds_dialog_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void _insertTabPage(int p_tcid, ds_dialog* p_page, char* p_label, DLGPROC p_tpproc, bool p_managed);

protected:
	DLGPROC	m_userproc;

	int				m_tabcount;
	struct TabData*	m_tabinfo[16];
};

__declspec(align(4)) class ds_filewindow
{
public:
	ds_filewindow(void);
	~ds_filewindow();

	OPENFILENAME* getOFN(void) { return &m_ofn; }
	char* getSelectFileName(void) { return (m_ofn.lpstrFile[0] != '\0') ? m_ofn.lpstrFile : NULL; }

	void setInitialDir(char* p_initdir);
	void setFilter(char* p_filter, int p_index = 0);

	bool openfile(const char* p_filter, DWORD p_flag = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST);
	bool savefile(const char* p_filter, const char* p_def_file, const char* p_def_ext, DWORD p_flag = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT);

private:
	OPENFILENAME	m_ofn;
};