#ifndef _message
#define _message

const char kSettingFileName[] = "setting.dat";
const char kPaletteDir[]      = "pal";

// Error Message
const char kErr_InitD3D[] =          "Failed to Initialize Direct3D.";
const char kErr_CallCOMFunc[] =      "Failed to %s. HRESULT=%08x";
const char kErr_FailedInitDSound[] = "Failed to Initialize DirectSound. HRESULT=%08x";

// For Debugger Only
const char kText_TitleStr_NoDDBFile[] = "No DDB File";

const char kMsg_FindItemOver[] = "一定数を超えたため中断します";
const char kMsg_FindItemNone[] = "一致する命令はありません";
const char kMsg_SourceFileNotFound[] = "ソースファイル%sは見つかりません";
const char kMsg_NoCurDDB[] = "カレントDDBが選択されていません";

const char kMenu_LineEdit[]    = "編集ダイアログを開く...\t<Enter>";
const char kMenu_FindRef[]     = "参照元を検索\t<Ctrl+R>";
const char kMenu_DispSrc[]     = "ソースを表示";
const char kMenu_DispAsm[]     = "アセンブリを表示";

#endif // _message
