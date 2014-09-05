/*---------------------------------------------------------------*/
/*                           ds_util.cpp                         */
/*---------------------------------------------------------------*/

#include "_prefix.h"

/*---------*/
/* include */
/*---------*/
#include "ds_util.h"
#include <iphlpapi.h>
#include <stdlib.h>
#include <math.h>

/*-------*/
/* const */
/*-------*/

/*--------*/
/* global */
/*--------*/
bool oldkey[256];	// グローバル変数は処理系に依らず自動で0クリアされるはず

/*------------*/
/* prototypes */
/*------------*/
void _strtrim(char* p_str, int p_param);

/*----------*/
/* function */
/*----------*/
bool ds_util::isPressKey(int p_vk)
{
	bool key = !!(GetAsyncKeyState(p_vk) & 0x8000);
	oldkey[p_vk] = key;
	return key;
}

bool ds_util::isDownKey(int p_vk)
{
	bool key = !!(GetAsyncKeyState(p_vk) & 0x8000);
	bool result = key && !oldkey[p_vk];
	oldkey[p_vk] = key;
	return result;
}

void ds_util::getMacAddress(int p_idx, char* p_buf)
{
	PIP_ADAPTER_INFO	adpinf;
	unsigned long		bufsize;
	
	memset(p_buf, 0, 6);

	GetAdaptersInfo(NULL, &bufsize);
	char* buf = new char[bufsize];
	adpinf = (PIP_ADAPTER_INFO)buf;

	if (GetAdaptersInfo(adpinf, &bufsize) == ERROR_SUCCESS)
	{
		for (int i = 0; adpinf != NULL; i++)
		{
			if (i == p_idx)
			{
				memcpy(p_buf, adpinf->Address, 6);
				break;
			}
			adpinf = adpinf->Next;
		}
	}
	delete buf;
}

DWORD ds_util::getSysDiskSN(void)
{
	char drivelabel[MAX_PATH];
	GetWindowsDirectory(drivelabel, MAX_PATH);
	drivelabel[3] = '\0';

	DWORD sn = 0xffffffff;
	if (GetDriveType(drivelabel) == DRIVE_FIXED)
	{
		GetVolumeInformation(
			drivelabel,	// ルートディレクトリ
			NULL,		// ボリューム名バッファ
			0,			// 名前バッファのサイズ
			&sn,		// ボリュームのシリアル番号
			0,			// ファイル名の最大の長さ
			0,			// ファイルシステムのオプション
			NULL,		// ファイルシステム名を格納するバッファ
			0);			 // ファイルシステム名を格納するバッファのサイズ
		//char str[9];
		//sprintf(str, "%08x", sn);
		//MessageBox(0, str, 0, 0);
	}
	return sn;
}

void ds_util::hex2str(const char* p_ptr, int p_size, char* p_str)
{
	for (int i = 0; i < p_size; i++)
	{
		sprintf(&p_str[i*2], "%02x", (BYTE)p_ptr[i]);
	}
}

int ds_util::str2hex(char* p_ptr, int p_size, const char* p_str, bool p_rev) {
	int actual_size = 0;
	int len = strlen(p_str);
	for (int i = 0; i < len; i++) {
		if (i >> 1 >= p_size) break;

		char c = *(p_str + (p_rev ? p_size * 2 - 1 - i : i));

		if ((i & 1) == 0) p_ptr[i >> 1] = 0;

		if ((i & 1) == p_rev) {
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				p_ptr[i >> 1] |=  (c - '0') << 4;
				break;
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				p_ptr[i >> 1] |=  (10 + c - 'a') << 4;
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				p_ptr[i >> 1] |=  (10 + c - 'A') << 4;
				break;
			default:
				return (actual_size + 1) >> 1;
			}
		} else {
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				p_ptr[i >> 1] |=  (c - '0');
				break;
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				p_ptr[i >> 1] |=  (10 + c - 'a');
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				p_ptr[i >> 1] |=  (10 + c - 'A');
				break;
			default:
				return (actual_size + 1) >> 1;
			}
		}
		actual_size++;
	}
	return (actual_size + 1) >> 1;
}

char* ds_util::strmid(char* p_dst, char* p_src, int p_st, int p_size)
{
	strncpy(p_dst, p_src + p_st, p_size);
	p_dst[p_size] = '\0';
	return p_dst;
}

void ds_util::strtrim(char* p_str)
{
	_strtrim(p_str, 0);
}

void ds_util::strtrimL(char* p_str)
{
	_strtrim(p_str, 2);
}

void ds_util::strtrimR(char* p_str)
{
	_strtrim(p_str, 1);
}

void _strtrim(char* p_str, int p_param)
{
	int st = 0, ed = (int)strlen(p_str) - 1;
	if (p_param != 1)
	{
		for (int i = 0; p_str[i] == ' ' || p_str[i] == '\t'; i++) st = i + 1;
	}
	if (p_param != 2)
	{
		for (int i = (int)strlen(p_str) - 1; p_str[i] == ' ' || p_str[i] == '\t'; i--) ed = i - 1;
	}
	for (int i = 0; p_str[i] != '\0'; i++)
	{
		p_str[i] = (i <= ed - st) ? p_str[i + st] : '\0';
	}
}

bool ds_util::strmatch(const char* p_str, const char* p_pat)
{
	// * : 0以上の任意の文字にマッチ
	// ? : 任意の1文字にマッチ

	bool asterisk = false;
	int i = 0;
	int j = 0;
	while (p_pat[i] != '\0') {
		if (p_str[j] == '\0') {
			return false;
		}

		bool lb1 = IS_MB_CHAR(p_pat[i]);
		bool lb2 = IS_MB_CHAR(p_str[j]);

		if (p_pat[i] == '?') {
			i++;
			j++;
			if (lb2) j++;
		} else if (p_pat[i] == '*') {
			asterisk = true;
			i++;
		} else if (!lb1 && !lb2 &&
			p_pat[i] == p_str[j])
		{
			// １バイト文字として一致
			asterisk = false;
			i++;
			j++;
		} else if (lb1 && lb2 &&
			*((const short*)(p_pat + i)) == *((const short*)(p_str + j)))
		{
			// ２バイト文字として一致
			asterisk = false;
			i += 2;
			j += 2;
		} else {
			// 一致しない
			if (asterisk == false) {
				i = 0;
			}
			j++;
			if (lb2) j++;
		}
	}
	// 特にパターンの前後に*を書かなくても部分一致として扱うので
	// パターンが終了した時点で差異がなければ一致とする
	return true;
}

void ds_util::strlower(char* p_str)
{
	for (int i = 0; p_str[i] != '\0'; i++)
	{
		if (p_str[i] >= 'A' && p_str[i] <= 'Z') p_str[i] += 'a' - 'A';
	}
}

char* ds_util::strdiv(char*& p_s1, char p_c1, char p_c2)
{
	// 文字列p_s1をp_c1の文字で区切る。p_c2で囲まれている範囲は区切らない。
	// p_c2が連続していたら通常の１つの文字として扱う。
	// 戻り値に抽出した文字が返り、p_s1はその次の文字列の先頭に更新される。
	char* head = p_s1;
	bool ignore = false;
	for (int i = 0;; i++)
	{
		if (p_s1[i] == p_c1 && ignore == false)
		{
			// 区切り文字を\0で置き換え、p_s1を更新
			p_s1[i] = '\0';
			p_s1 = p_s1 + i + 1;
			break;
		}
		if (p_s1[i] == '\0')
		{
			// 終端まで到達したら、p_s1は最後のNULL文字を指す。
			p_s1 = p_s1 + i;
			break;
		}
		if (p_s1[i] == p_c2) {
			if (p_s1[i + 1] != p_c2) {
				ignore = !ignore;
			} else {
				// ２つ目を無視する
				i++;
			}
		}
	}
	return head;
}

char* ds_util::strreplace(char* p_dst, int p_dstsz, const char* p_src, int p_srcsz, const char* p_str1, const char* p_str2) {

	int len1 = strlen(p_str1);
	int len2 = strlen(p_str2);

	int i = 0, j = 0;
	while (p_src[j] != '\0' && j < p_srcsz) {
		if (memcmp(&p_src[j], p_str1, len1) == 0) {
			if (i + len2 - len1 > p_dstsz) {
				break;	// バッファオーバー
			}
			// 置換え
			strcpy(&p_dst[i], p_str2);
			i += len2;
			j += len1;
			continue;
		}
		if (i + 1 > p_dstsz) {
			break;	// バッファオーバー
		}
		p_dst[i++] = p_src[j++];
	}
	p_dst[i] = '\0';
	return p_dst;
}

int ds_util::fsize32(FILE* p_fp)
{
	fpos_t pos, old;
	fgetpos(p_fp, &old);
	fseek(p_fp, 0, SEEK_END);
	fgetpos(p_fp, &pos);
	fsetpos(p_fp, &old);
	return (int)pos;
}

int ds_util::fgetpos32(FILE* p_fp)
{
	fpos_t fpos;
	fgetpos(p_fp, &fpos);
	return (int)fpos;
}

void ds_util::getParentDirectory(const char* p_path, char* p_out)
{
	const char* p1 = strrchr(p_path, '\\');
	const char* p2 = strrchr(p_path, '/');

	if (p1 > p2)		{ strncpy(p_out, p_path, p1 - p_path); p_out[p1 - p_path] = '\0'; }
	else if (p2 > p1)	{ strncpy(p_out, p_path, p2 - p_path); p_out[p2 - p_path] = '\0'; }
	else				strcpy(p_out, p_path);
}

void ds_util::getFileName(const char* p_path, char* p_out)
{
	const char* p1 = strrchr(p_path, '\\');
	const char* p2 = strrchr(p_path, '/');

	int len = strlen(p_path);
	if (p1 > p2)		{ strncpy(p_out, p1 + 1, p_path + len - p1 - 1); p_out[p_path + len - p1 - 1] = '\0'; }
	else if (p2 > p1)	{ strncpy(p_out, p2 + 1, p_path + len - p2 - 1); p_out[p_path + len - p2 - 1] = '\0'; }
	else				strcpy(p_out, p_path);
}

void ds_util::getRelativePath(const char* p_path1, const char* p_path2, char* p_out, bool p_matchcase)
{
	char tmp1[1024], tmp2[1024];
	strncpy(tmp1, p_path1, 1024);
	strncpy(tmp2, p_path2, 1024);
	
	// パスの正規化
	int len1 = strlen(tmp1);
	int len2 = strlen(tmp2);
	for (int i = 0; i < len1; i++)
	{
		if (tmp1[i] == '\\') tmp1[i] = '/';
		else if (p_matchcase == false && tmp1[i] >= 'A' && tmp1[i] <= 'Z') tmp1[i] += 'a' - 'A';
	}
	for (int i = 0; i < len2; i++)
	{
		if (tmp2[i] == '\\') tmp2[i] = '/';
		else if (p_matchcase == false && tmp2[i] >= 'A' && tmp2[i] <= 'Z') tmp2[i] += 'a' - 'A';
	}
	
	// 共通部分まで取り出し
	int baselen = 0;
	for (int i = 0; tmp1[i] == tmp2[i]; i++)
	{
		if (tmp1[i] == '/') baselen = i + 1;
	}

	if (baselen == 0)
	{
		// 完全不一致なので正規化済みの絶対パスを返す
		strcpy(p_out, tmp2);
	}
	else
	{
		p_out[0] = '\0';
		// p_path1の残りからディレクトリの数だけ"../"を付加する
		for (int i = baselen; i < len1; i++)
		{
			if (tmp1[i] == '/') strcat(p_out, "../");
		}
		// p_path2の残りを付加する
		strcat(p_out, &tmp2[baselen]);
	}
}

void ds_util::TextOutColor(HDC p_hdc, int p_x, int p_y, char* p_str, COLORREF p_color)
{
	char		buf[1024];
	bool		setmode = false;
	bool		textout = false;
	bool		done = false;
	COLORREF	color = p_color;

	COLORREF savecol = GetTextColor(p_hdc);

	int		i = 0, j = 0, k = 0;
	while (!done)
	{
		if (p_str[i] == '\0')
		{
			buf[j++] = '\0';
			SetTextColor(p_hdc, color);
			textout = true;
			done = true;
		}
		else if (p_str[i] == '\r' || p_str[i] == '\n' || p_str[i] == '\t')
		{
		}
		else if (setmode)
		{
			if (p_str[i] == '@') buf[j++] = '@';
			else
			{
				SetTextColor(p_hdc, color);
				textout = true; 
				switch (p_str[i])
				{
				case 'x': color = RGB(  0,  0,  0);	break;	// 黒
				case 'd': color = RGB(128,128,128);	break;	// 灰
				case 'w': color = RGB(255,255,255);	break;	// 白
				case 'r': color = RGB(255,  0,  0); break;	// 赤
				case 'g': color = RGB(  0,255,  0); break;	// 緑
				case 'b': color = RGB(  0,  0,255); break;	// 青
				case 'c': color = RGB(  0,255,255); break;	// シアン
				case 'y': color = RGB(255,255,  0);	break;	// イエロー
				case 'm': color = RGB(255,  0,255);	break;	// マゼンタ
				case 'R': color = RGB(128,  0,  0); break;	// 暗赤
				case 'G': color = RGB(  0,128,  0); break;	// 暗緑
				case 'B': color = RGB(  0,  0,128); break;	// 暗青
				case 'o': color = RGB(255,128,  0);	break;	// 橙
				case 'f': color = RGB(  0,128,128);	break;	// 青緑
				case 'p': color = RGB(128,  0,255);	break;	// 紫
				default: break;
				}
			}
			setmode = false;
		}
		else // setmode == false
		{
			if (p_str[i] == '@')
			{
				setmode = true;
			}
			else
			{
				buf[j++] = p_str[i];
			}
		}

		if (textout && j - k > 0)
		{
			SIZE size;
			GetTextExtentPoint32(p_hdc, buf, k, &size);
			TextOut(p_hdc, p_x + size.cx, p_y, &buf[k], j - k);
			k = j;
			textout = false;
		}
		textout = false;
		i++;
	}

	SetTextColor(p_hdc, savecol);
}

int ds_util::getfps(void)
{
	timeBeginPeriod(1);

	LARGE_INTEGER freq;
	int time;
	if (QueryPerformanceFrequency(&freq))
	{
		static LARGE_INTEGER last = { 0 };
		LARGE_INTEGER now;
		
		QueryPerformanceCounter(&now);
		time = (int)((now.QuadPart - last.QuadPart) * 1000000 / freq.QuadPart);
		last = now;
	}
	else
	{
		static DWORD last = 0;
		DWORD now;

		now = timeGetTime();
		time = (now - last) * 1000;
		last = now;
	}

	int fps;
	if (time == 0)
	{
		fps = 0;
	}
	else
	{
		// 四捨五入
		int mod = 1000000 % time;
		fps = 1000000 / time + ((mod >= time / 2) ? 1 : 0);
	}
	return fps;
}

void ds_util::sync60fps(void)
{
	timeBeginPeriod(1);

	LARGE_INTEGER freq;
	
	if (QueryPerformanceFrequency(&freq))
	{
		static LARGE_INTEGER last = { 0 };
		LARGE_INTEGER now;

		while (1)
		{
			QueryPerformanceCounter(&now);
			if (last.QuadPart == 0) break;	// 初回はスルー
			int time = (int)((now.QuadPart - last.QuadPart) * 1000000 / freq.QuadPart);
			if (time > 16667) break;
			else if (time < 15000) Sleep(1);
		}
		last = now;
	}
	else
	{
		static DWORD last = 0;
		DWORD now;

		while (1)
		{
			now = timeGetTime();
			if (last == 0) break;			// 初回はスルー
			int time = (now - last) * 1000;
			if (time > 16667) break;
			else if (time < 15000) Sleep(1);
		}
		last = now;
	}
}

DWORD ds_util::atox(char* p_str, int p_radix)
{
	char* endstr;
	return strtoul(p_str, &endstr, p_radix);
}

void ds_util::normalizeRect(LPRECT p_rect)
{
	if (p_rect->left > p_rect->right)
	{
		int tmp = p_rect->left;
		p_rect->left = p_rect->right;
		p_rect->right = tmp;
	}
	if (p_rect->top > p_rect->bottom)
	{
		int tmp = p_rect->top;
		p_rect->top = p_rect->bottom;
		p_rect->bottom = tmp;
	}
}

int ds_util::createArgString(char* p_cmdline, char**& p_argv)
{
	// 文字列引数を解析し、p_argvに切り分ける
	// ポインタ配列を必要に応じてアロケートするので呼び出し元で
	// 使用後にdeleteArgStringする必要がある。
	int argmax = 4;
	int argc = 0;
	p_argv = (char**)malloc(sizeof(char*) * argmax);

	bool	inDoubleQuote = false;
	char*	strhead = NULL;
	int		strsize = 0;
	for (char* p = p_cmdline; *p != '\0'; p++)
	{
		if (IsDBCSLeadByte(*p) == false)
		{
			// Single Byte
			if (*p == ' ')
			{
				// ダブルクォートで囲まれている場合は半角スペースを見つけても引数の区切りとしない
				if (inDoubleQuote == false)
				{
					if (strhead)
					{
						// サイズ拡張
						if (argc >= argmax)
						{
							int oldargmax = argmax;
							argmax += 4;
							char** newbuf = (char**)malloc(sizeof(char*) * argmax);
							memcpy(newbuf, p_argv, sizeof(char*) * oldargmax);
							delete[] p_argv;
							p_argv = newbuf;
						}

						p_argv[argc] = (char*)malloc(strsize + 1);
						strncpy(p_argv[argc], strhead, strsize);
						p_argv[argc][strsize] = '\0';
						argc++;

						strhead = NULL;
						strsize = 0;
					}
				}
				else strsize++;
			}
			else
			{
				if (strhead == NULL) strhead = p;
				if (*p == '\"') inDoubleQuote = !inDoubleQuote;
				strsize++;
			}
		}
		else
		{
			// Multi Bytes
			if (strhead == NULL) strhead = p;
			strsize += 2;
			p++;
		}
	}
	if (strhead)
	{
		// サイズ拡張
		if (argc >= argmax)
		{
			int oldargmax = argmax;
			argmax += 4;
			char** newbuf = (char**)malloc(sizeof(char*) * argmax);
			memcpy(newbuf, p_argv, sizeof(char*) * oldargmax);
			delete[] p_argv;
			p_argv = newbuf;
		}

		p_argv[argc] = (char*)malloc(strsize + 1);
		strncpy(p_argv[argc], strhead, strsize);
		p_argv[argc][strsize] = '\0';
		argc++;
	}
	return argc;
}

void ds_util::deleteArgString(int p_argc, char**& p_argv)
{
	for (int i = 0; i < p_argc; i++)
	{
		if (p_argv[i]) delete p_argv[i];
	}
	if (p_argv)
	{
		delete[] p_argv;
		p_argv = NULL;
	}
}

int ds_util::findArg(int p_argc, char** p_argv, char* p_str)
{
	for (int i = 0; i < p_argc; i++)
	{
		if (strcmp(p_argv[i], p_str) == 0)
		{
			return i;
		}
	}
	return -1;
}

int ds_util::findArgValue(int p_argc, char** p_argv, char* p_str, int p_valueIdx)
{
	int valueIdx = 0;
	bool find = false;
	for (int i = 0; i < p_argc; i++)
	{
		if (strcmp(p_argv[i], p_str) == 0)
		{
			find = true;
		}
		else if (find)
		{
			if (strlen(p_argv[i]) == 2 && p_argv[i][0] == '-')
			{
				// 値の検索中に先頭が'-'の２文字の文字列があったら次の引数とみなし、検索を終了する。
				break;
			}
			if (valueIdx++ == p_valueIdx)
			{
				return i;
			}
		}
	}
	return -1;
}

WORD ds_util::getcw(void)
{
	WORD cw;
	_asm fstcw cw;
	return cw;
}

void ds_util::setcw(WORD p_cw)
{
	_asm fldcw p_cw;
}

void ds_util::RGB2HSB(int p_r, int p_g, int p_b, float &p_hue, float &p_sat, float &p_bri)
{
	int max = max(p_r, max(p_g, p_b));
	int min = min(p_r, min(p_g, p_b));
	float diff = (float)(max - min);

	if (diff > 0)
	{
		if (max == p_r)			p_hue = 60.0f * (float)(p_g - p_b) / diff;
		else if (max == p_g)	p_hue = 60.0f * (float)(p_b - p_r) / diff + 120.0f;
		else if (max == p_b)	p_hue = 60.0f * (float)(p_r - p_g) / diff + 240.0f;
	}
	else p_hue = 0.0f;

	p_sat = (max > 0) ? (diff / (float)max) : 0.0f;
	p_bri = (float)max / 255.0f;
}

void ds_util::HSB2RGB(float p_hue, float p_sat, float p_bri, int &p_r, int &p_g, int &p_b)
{
	if (p_sat == 0.0)
	{
		p_r = p_g = p_b = (int)(p_bri * 255.0f);
	}
	else
	{
		int		hi	= abs((int)p_hue) / 60 % 6;
		float	f	= p_hue / 60.0f - (float)hi;
		float	p	= p_bri * (1.0f - p_sat);
		float	q	= p_bri * (1.0f - p_sat * f);
		float	t	= p_bri * (1.0f - p_sat * (1.0f - f));

		switch (hi)
		{
		case 0:
			p_r = (int)(255.0f * p_bri);
			p_g = (int)(255.0f * t);
			p_b = (int)(255.0f * p);
			break;
		case 1:
			p_r = (int)(255.0f * q);
			p_g = (int)(255.0f * p_bri);
			p_b = (int)(255.0f * p);
			break;
		case 2:
			p_r = (int)(255.0f * p);
			p_g = (int)(255.0f * p_bri);
			p_b = (int)(255.0f * t);
			break;
		case 3:
			p_r = (int)(255.0f * p);
			p_g = (int)(255.0f * q);
			p_b = (int)(255.0f * p_bri);
			break;
		case 4:
			p_r = (int)(255.0f * t);
			p_g = (int)(255.0f * p);
			p_b = (int)(255.0f * p_bri);
			break;
		case 5:
			p_r = (int)(255.0f * p_bri);
			p_g = (int)(255.0f * p);
			p_b = (int)(255.0f * q);
			break;
		}
	}
}

void ds_util::rgb2yuvf(const double p_r, const double p_g, const double p_b, double &p_y, double &p_u, double &p_v) {
	p_y =  0.2990 * p_r + 0.5870 * p_g + 0.1140 * p_b;
	p_u = -0.1684 * p_r - 0.3316 * p_g + 0.5000 * p_b;
	p_v =  0.5000 * p_r - 0.4187 * p_g - 0.0813 * p_b;
}

void ds_util::rgb2yuv(const long p_r, const long p_g, const long p_b, long &p_y, long &p_u, long &p_v) {
	static const int _0_2990 = (int)(0.2990 * 1024.0);
	static const int _0_5870 = (int)(0.5870 * 1024.0);
	static const int _0_1140 = (int)(0.1140 * 1024.0);
	static const int _0_1684 = (int)(0.1684 * 1024.0);
	static const int _0_3316 = (int)(0.3316 * 1024.0);
	static const int _0_5000 = (int)(0.5000 * 1024.0);
	static const int _0_4187 = (int)(0.4187 * 1024.0);
	static const int _0_0813 = (int)(0.0813 * 1024.0);

	p_y = ( _0_2990 * p_r + _0_5870 * p_g + _0_1140 * p_b) >> 10;
	p_u = (-_0_1684 * p_r - _0_3316 * p_g + _0_5000 * p_b) >> 10;
	p_v = ( _0_5000 * p_r - _0_4187 * p_g - _0_0813 * p_b) >> 10;
}

void ds_util::yuv2rgbf(const double p_y, const double p_u, const double p_v, double &p_r, double &p_g, double &p_b) {
	p_r = p_y + 1.4020 * p_v;
	p_g = p_y - 0.3441 * p_u - 0.7139 * p_v;
	p_b = p_y + 1.7718 * p_u - 0.0012 * p_v;
}

void ds_util::yuv2rgb(const long p_y, const long p_u, const long p_v, long &p_r, long &p_g, long &p_b) {
	static const int _1_4020 = (int)(1.4020 * 1024.0);
	static const int _0_3441 = (int)(0.3441 * 1024.0);
	static const int _0_7139 = (int)(0.7139 * 1024.0);
	static const int _1_7718 = (int)(1.7718 * 1024.0);
	static const int _0_0012 = (int)(0.0012 * 1024.0);

	p_r = p_y + ((_1_4020 * p_v) >> 10);
	p_g = p_y - ((_0_3441 * p_u + _0_7139 * p_v) >> 10);
	p_b = p_y + ((_1_7718 * p_u + _0_0012 * p_v) >> 10);
	if (p_r > 255) p_r = 255;
	if (p_g > 255) p_g = 255;
	if (p_b > 255) p_b = 255;
	if (p_r < 0) p_r = 0;
	if (p_g < 0) p_g = 0;
	if (p_b < 0) p_b = 0;
}

void ds_util::rgb2ycbcr(const long p_r, const long p_g, const long p_b, long &p_y, long &p_u, long &p_v) {
	static const int _0_257 = (int)(0.257 * 1024.0);
	static const int _0_504 = (int)(0.504 * 1024.0);
	static const int _0_098 = (int)(0.098 * 1024.0);
	static const int _0_148 = (int)(0.148 * 1024.0);
	static const int _0_291 = (int)(0.291 * 1024.0);
	static const int _0_439 = (int)(0.439 * 1024.0);
	static const int _0_368 = (int)(0.368 * 1024.0);
	static const int _0_071 = (int)(0.071 * 1024.0);

	p_y = ( _0_257 * p_r + _0_504 * p_g + _0_098 * p_b + 16) >> 10;
	p_u = (-_0_148 * p_r - _0_291 * p_g + _0_439 * p_b + 128) >> 10;
	p_v = ( _0_439 * p_r - _0_368 * p_g - _0_071 * p_b + 128) >> 10;
}

void ds_util::ycbcr2rgb(const long p_y, const long p_u, const long p_v, long &p_r, long &p_g, long &p_b) {
	static const int _1_164 = (int)(1.164 * 1024.0);
	static const int _1_596 = (int)(1.596 * 1024.0);
	static const int _0_391 = (int)(0.391 * 1024.0);
	static const int _0_813 = (int)(0.813 * 1024.0);
	static const int _2_018 = (int)(2.018 * 1024.0);

	p_r = (_1_164 * (p_y - 16) + _1_596 * (p_v - 128)) >> 10;
	p_g = (_1_164 * (p_y - 16) - _0_391 * (p_u - 128) - _0_813 * (p_v - 128)) >> 10;
	p_b = (_1_164 * (p_y - 16) + _2_018 * (p_u - 128)) >> 10;
	if (p_r > 255) p_r = 255;
	if (p_g > 255) p_g = 255;
	if (p_b > 255) p_b = 255;
	if (p_r < 0) p_r = 0;
	if (p_g < 0) p_g = 0;
	if (p_b < 0) p_b = 0;
}

void ds_util::gauss_jordan(double* m, int p_row, int p_col)
{
	// ガウス・ジョルダン法により連立方程式を解く
	for (int i = 0; i < p_row; i++)
	{
		double tmp;

		// ピボット選択
		int		maxabsidx = i;
		double	maxabsval = fabs(m[i*p_col + i]);
		for (int j = i+1; j < p_row ; j++)
		{
			if (fabs(m[j*p_col + i]) > maxabsval)
			{
				maxabsidx = j;
				maxabsval = fabs(m[j*p_col + i]);
			}
		}
		// 絶対値の大きいものがあれば行を入れ替え
		if (maxabsidx != i)
		{
			for (int k = 0; k < p_col; k++)
			{
				tmp = m[maxabsidx*p_col + k];
				m[maxabsidx*p_col + k] = m[i*p_col + k];
				m[i*p_col + k] = tmp;
			}
		}
		// 無効な列があれば、スキップ
		if (maxabsval > 0.00001)
		{
			// 対角成分の要素で行全体を割り、対角成分を1にする
			tmp = m[i*p_col + i];
			for (int k = i; k < p_col; k++)
			{
				m[i*p_col + k] /= tmp;
			}

			// i列目の対角成分以外が0になるように
			for (int j = 0; j < p_row; j++)
			{
				if (i == j) continue;

				tmp = m[j*p_col + i];
				for (int k = i; k < p_col; k++)
				{
					m[j*p_col + k] -= tmp * m[i*p_col + k];
				}
			}
		}
	}
}

double ds_util::spline(double* p_xa, double* p_ya, int p_len, double p_x)
{
	// Sj(x)   =  aj(x-Xj)^3 +  bj(x-Xj)^2 + cj(x-Xj) + dj
	// Sj(x)'  = 3aj(x-Xj)^2 + 2bj(x-Xj)   + cj
	// Sj(x)'' = 6aj(x-Xj)   + 2bj
	//
	// [1] Sj(Xj)   = dj = Yj
	//
	// [2] Sj(Xj)'' = 2bj
	//     Sj(Xj)'' = Uj とおく
	//           bj = Uj / 2
	//
	// [3] Sj(Xj+1)'' = 6a(Xj+1 - Xj) + Uj
	//     Sj(Xj+1)'' = Uj+1 とおく
	//             aj = (Uj+1 - Uj) / 6(Xj+1 - Xj)
	//
	// [4] Sj(Xj+1)  = Yj+1 =                             aj(Xj+1 - Xj)^3 +        bj(Xj+1 - Xj)^2 + cj(Xj+1 - Xj) + dj
	//                 Yj+1 =  (Uj+1 - Uj) / 6(Xj+1 - Xj) * (Xj+1 - Xj)^3 + Uj / 2 * (Xj+1 - Xj)^2 + cj(Xj+1 - Xj) + Yj
	//        cj(Xj+1 - Xj) = -(Uj+1 - Uj) / 6(Xj+1 - Xj)^2               - Uj / 2 * (Xj+1 - Xj)^2                 - Yj + Yj+1
	//                   cj = (Yj+1 - Yj) / (Xj+1 - Xj) - 1/6 * (Xj+1 - Xj) * (Uj+1 + 2 * Uj)
	//
	// [5] Sj'(x) = Sj+1'(x) なので
	//     Sj'(Xj+1) = Sj+1'(Xj+1)
	//     3aj(Xj+1 - Xj)^2 + 2bj(Xj+1 - Xj) + cj = 3aj+1(Xj+1 - Xj+1)^2 + 2bj+1(Xj+1 - Xj+1) + cj+1
	//     3aj(Xj+1 - Xj)^2 + 2bj(Xj+1 - Xj) + cj = cj+1
	//     ここで Hj = Xj+1 - Xj, Vj = Yj+1 - Yj とおくと
	//     3/6 * (Uj+1 - Uj) * Hj + Uj * Hj + Vj / Hj - 1/6 * Hj * (Uj+1 + 2 * Uj) = cj+1
	//     [3Uj+1 - 3Uj - Uj+1 - 2Uj + 6Uj] * Hj / 6 + Vj / Hj = cj+1
	//                         [2Uj+1 + Uj] * Hj / 6 + Vj / Hj = cj+1
	//                   Uj * Hj / 6 + Uj+1 * Hj / 3 + Vj / Hj = (Yj+2 - Yj+1) / (Xj+2 - Xj+1) - 1/6 * (Xj+2 - Xj+1) * (Uj+2 + 2 * Uj+1)
	//                   Uj * Hj / 6 + Uj+1 * Hj / 3 + Vj / Hj =          Vj+1 / Hj+1          - Uj+2 * Hj+1 / 6 - Uj+1 * Hj+1 / 3
	//             Uj+2 * Hj+1 * Uj+1 * 2(Hj + Hj+1) + Uj * Hj = 6(Vj*1 / Hj+1 - Vj / Hj)
	//
	// [6] 自然スプラインのため両端の２次導関数の値をU0 = U5 = 0 とする
	//     | H0     2(H0+H1) H1                            ||0 |   |6 * (V1 / H1 - V0 / H0)|
	//     |        H1       2(H1+H2) H2                   ||U1|   |6 * (V2 / H2 - V1 / H1)|
	//     |                 H2       2(H2+H3) H3          ||U2| = |6 * (V3 / H3 - V2 / H2)|
	//     |                          H3       2(H3+H4) H4 ||U3|   |6 * (V4 / H4 - V3 / H3)|
	//                                                      |U4|
	//                                                      |0 |
	//
	//     | 2(H0+H1) H1                         ||U1|   |6 * (V1 / H1 - V0 / H0)|
	//     | H1       2(H1+H2) H2                ||U2|   |6 * (V2 / H2 - V1 / H1)|
	//     |          H2       2(H2+H3) H3       ||U3| = |6 * (V3 / H3 - V2 / H2)|
	//     |                   H3       2(H3+H4) ||U4|   |6 * (V4 / H4 - V3 / H3)|
	
	if (p_len == 1)			return p_ya[0];
	else if (p_len == 2)	return (p_x - p_xa[0]) * (p_ya[1] - p_ya[0]) / (p_xa[1] - p_xa[0]) + p_ya[0];

	// p_xの位置から区間を選択
	int idx;
	for (int i = 1; i < p_len; i++)
	{
		idx = i - 1;
		if (p_xa[i] > p_x) break;
	}

	double xa[4], ya[4];
	if (idx == 0)
	{
		xa[0] = p_xa[0];
		ya[0] = p_ya[0];
	}
	else
	{
		xa[0] = p_xa[idx-1];
		ya[0] = p_ya[idx-1];
	}
	xa[1] = p_xa[idx];
	ya[1] = p_ya[idx];
	xa[2] = p_xa[idx+1];
	ya[2] = p_ya[idx+1];
	if (idx == p_len - 2)
	{
		xa[3] = p_xa[idx+1];
		ya[3] = p_ya[idx+1];
	}
	else
	{
		xa[3] = p_xa[idx+2];
		ya[3] = p_ya[idx+2];
	}

	double m[4][5];
	m[0][0] = 2.0 * (xa[1] - xa[0]);
	m[0][1] = xa[1] - xa[0];
	m[0][2] = 0.0;
	m[0][3] = 0.0;
	if (fabs(xa[1] - xa[0]) > 0.000001)	m[0][4] = 6.0 * (ya[1] - ya[0]) / (xa[1] - xa[0]);
	else								m[0][4] = 0.0;

	m[1][0] = xa[1] - xa[0];
	m[1][1] = 2.0 * (xa[2] - xa[0]);
	m[1][2] = xa[2] - xa[1];
	m[1][3] = 0.0;
	if (fabs(xa[1] - xa[0]) > 0.000001)	m[1][4] = 6.0 * ((ya[2] - ya[1]) / (xa[2] - xa[1]) - (ya[1] - ya[0]) / (xa[1] - xa[0]));
	else								m[1][4] = 6.0 * (ya[2] - ya[1]) / (xa[2] - xa[1]);

	m[2][0] = 0.0;
	m[2][1] = xa[2] - xa[1];
	m[2][2] = 2.0 * (xa[3] - xa[1]);
	m[2][3] = xa[3] - xa[2];
	if (fabs(xa[3] - xa[2]) > 0.000001)	m[2][4] = 6.0 * ((ya[3] - ya[2]) / (xa[3] - xa[2]) - (ya[2] - ya[1]) / (xa[2] - xa[1]));
	else								m[2][4] = -6.0 * (ya[2] - ya[1]) / (xa[2] - xa[1]);

	m[3][0] = 0.0;
	m[3][1] = 0.0;
	m[3][2] = xa[3] - xa[2];
	m[3][3] = 2.0 * (xa[3] - xa[2]);
	if (fabs(xa[3] - xa[2]) > 0.000001)	m[3][4] = -6.0 * (ya[3] - ya[2]) / (xa[3] - xa[2]);
	else								m[3][4] = 0.0;

	ds_util::gauss_jordan(&m[0][0], 4, 5);

	double a = (m[2][4] - m[1][4]) / (6 * (xa[2] - xa[1]));
	double b = m[1][4] * 0.5;
	double c = (ya[2] - ya[1]) / (xa[2] - xa[1]) - (xa[2] - xa[1]) * (m[2][4] + 2 * m[1][4]) / 6;
	double d = ya[1];

	double x = p_x - xa[1];
	return a*x*x*x + b*x*x + c*x + d;
}

#ifdef _DEBUG
void ds_util::DBGOUTA(char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	
	char buf[1024];
	vsprintf(buf, fmt, ap);

	OutputDebugString(buf);
	
	//FILE *fp = fopen("debug.txt", "a");
	//if (fp)
	//{
	//	fprintf(fp, buf);
	//	fclose(fp);
	//}

	va_end(ap);
}

void ds_util::DBGOUTW(wchar_t* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	
	wchar_t buf[1024];
	vswprintf(buf, fmt, ap);

	OutputDebugStringW(buf);
	
	//FILE *fp = fopen("debug.txt", "a");
	//if (fp)
	//{
	//	fprintf(fp, buf);
	//	fclose(fp);
	//}

	va_end(ap);
}
#endif

#include "_postfix.h"
