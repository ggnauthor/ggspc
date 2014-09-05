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

const char kMsg_FindItemOver[] = "��萔�𒴂������ߒ��f���܂�";
const char kMsg_FindItemNone[] = "��v���閽�߂͂���܂���";
const char kMsg_SourceFileNotFound[] = "�\�[�X�t�@�C��%s�͌�����܂���";
const char kMsg_NoCurDDB[] = "�J�����gDDB���I������Ă��܂���";

const char kMenu_LineEdit[]    = "�ҏW�_�C�A���O���J��...\t<Enter>";
const char kMenu_FindRef[]     = "�Q�ƌ�������\t<Ctrl+R>";
const char kMenu_DispSrc[]     = "�\�[�X��\��";
const char kMenu_DispAsm[]     = "�A�Z���u����\��";

#endif // _message
