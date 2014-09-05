///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "ggscfg_ColorEditSheet.h"
#include "ggscfg_GeneralSheet.h"
#include "ggscfg_NetworkSheet.h"

#include "ggscfg_gui.h"

///////////////////////////////////////////////////////////////////////////

ConfigDialog::ConfigDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 580,480 ), wxSize( 700,700 ) );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_notebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxSize( 111,111 ), 0 );
	m_panel_general = new ggscfg_GeneralSheet( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_notebook->AddPage( m_panel_general, wxT("一般設定"), true );
	m_panel_network = new ggscfg_NetworkSheet( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_notebook->AddPage( m_panel_network, wxT("ネットワーク"), false );
	m_panel_color_edit = new ggscfg_ColorEditSheet( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_notebook->AddPage( m_panel_color_edit, wxT("カラーエディット"), false );
	
	bSizer1->Add( m_notebook, 1, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer32->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_button_ok = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer32->Add( m_button_ok, 0, wxALL, 5 );
	
	m_button_cancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer32->Add( m_button_cancel, 0, wxALL, 5 );
	
	bSizer1->Add( bSizer32, 0, wxEXPAND, 5 );
	
	this->SetSizer( bSizer1 );
	this->Layout();
	
	// Connect Events
	m_button_ok->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ConfigDialog::on_ok ), NULL, this );
	m_button_cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ConfigDialog::on_cancel ), NULL, this );
}

ConfigDialog::~ConfigDialog()
{
	// Disconnect Events
	m_button_ok->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ConfigDialog::on_ok ), NULL, this );
	m_button_cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ConfigDialog::on_cancel ), NULL, this );
	
}

GeneralSheet::GeneralSheet( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	wxFlexGridSizer* fgSizerRoot;
	fgSizerRoot = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerRoot->SetFlexibleDirection( wxBOTH );
	fgSizerRoot->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizerLeft;
	fgSizerLeft = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizerLeft->SetFlexibleDirection( wxBOTH );
	fgSizerLeft->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerGraphic;
	sbSizerGraphic = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("グラフィック") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizerDisplayDevice;
	fgSizerDisplayDevice = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizerDisplayDevice->SetFlexibleDirection( wxBOTH );
	fgSizerDisplayDevice->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText11 = new wxStaticText( this, wxID_ANY, wxT("デバイス"), wxDefaultPosition, wxSize( 56,-1 ), 0 );
	m_staticText11->Wrap( -1 );
	fgSizerDisplayDevice->Add( m_staticText11, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	wxArrayString m_choice_display_deviceChoices;
	m_choice_display_device = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 130,-1 ), m_choice_display_deviceChoices, 0 );
	m_choice_display_device->SetSelection( 0 );
	fgSizerDisplayDevice->Add( m_choice_display_device, 0, wxALL, 2 );
	
	sbSizerGraphic->Add( fgSizerDisplayDevice, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerDisplayDevice1;
	fgSizerDisplayDevice1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizerDisplayDevice1->SetFlexibleDirection( wxBOTH );
	fgSizerDisplayDevice1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText111 = new wxStaticText( this, wxID_ANY, wxT("画面モード"), wxDefaultPosition, wxSize( 56,-1 ), 0 );
	m_staticText111->Wrap( -1 );
	fgSizerDisplayDevice1->Add( m_staticText111, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	wxString m_choice_display_modeChoices[] = { wxT("ウインドウ"), wxT("フルスクリーン（協調）"), wxT("フルスクリーン（排他）") };
	int m_choice_display_modeNChoices = sizeof( m_choice_display_modeChoices ) / sizeof( wxString );
	m_choice_display_mode = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 130,-1 ), m_choice_display_modeNChoices, m_choice_display_modeChoices, 0 );
	m_choice_display_mode->SetSelection( 0 );
	fgSizerDisplayDevice1->Add( m_choice_display_mode, 0, wxALL, 2 );
	
	sbSizerGraphic->Add( fgSizerDisplayDevice1, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerColorDepth;
	fgSizerColorDepth = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizerColorDepth->SetFlexibleDirection( wxBOTH );
	fgSizerColorDepth->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("表示色数"), wxDefaultPosition, wxSize( 56,-1 ), 0 );
	m_staticText1->Wrap( -1 );
	fgSizerColorDepth->Add( m_staticText1, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	wxString m_choice_color_depthChoices[] = { wxT("32bit (A8R8G8B8)"), wxT("16bit (A1R5G5B5)") };
	int m_choice_color_depthNChoices = sizeof( m_choice_color_depthChoices ) / sizeof( wxString );
	m_choice_color_depth = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 130,-1 ), m_choice_color_depthNChoices, m_choice_color_depthChoices, 0 );
	m_choice_color_depth->SetSelection( 0 );
	fgSizerColorDepth->Add( m_choice_color_depth, 0, wxALL, 2 );
	
	sbSizerGraphic->Add( fgSizerColorDepth, 1, wxEXPAND, 2 );
	
	m_checkBox_wait_vsync = new wxCheckBox( this, wxID_ANY, wxT("垂直同期有効"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerGraphic->Add( m_checkBox_wait_vsync, 0, wxALL, 2 );
	
	m_checkBox_frame_skip = new wxCheckBox( this, wxID_ANY, wxT("フレームスキップ有効"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerGraphic->Add( m_checkBox_frame_skip, 0, wxALL, 2 );
	
	m_checkBox_keep_aspect_ratio = new wxCheckBox( this, wxID_ANY, wxT("アスペクト比固定"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerGraphic->Add( m_checkBox_keep_aspect_ratio, 0, wxALL, 2 );
	
	fgSizerLeft->Add( sbSizerGraphic, 1, wxALL|wxEXPAND, 2 );
	
	wxStaticBoxSizer* sbSizerSound;
	sbSizerSound = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("サウンド") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizerVolume;
	fgSizerVolume = new wxFlexGridSizer( 1, 4, 0, 0 );
	fgSizerVolume->SetFlexibleDirection( wxBOTH );
	fgSizerVolume->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText_bgm = new wxStaticText( this, wxID_ANY, wxT("BGM"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_bgm->Wrap( -1 );
	fgSizerVolume->Add( m_staticText_bgm, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_checkBox_bgm = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_bgm->SetValue(true); 
	fgSizerVolume->Add( m_checkBox_bgm, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_slider_bgm = new wxSlider( this, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxSize( -1,20 ), wxSL_HORIZONTAL );
	fgSizerVolume->Add( m_slider_bgm, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrl_bgm = new wxTextCtrl( this, wxID_ANY, wxT("255"), wxDefaultPosition, wxSize( 32,-1 ), 0 );
	fgSizerVolume->Add( m_textCtrl_bgm, 0, wxALL, 2 );
	
	m_staticText_se = new wxStaticText( this, wxID_ANY, wxT("SE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_se->Wrap( -1 );
	fgSizerVolume->Add( m_staticText_se, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_checkBox_se = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_se->SetValue(true); 
	fgSizerVolume->Add( m_checkBox_se, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_slider_se = new wxSlider( this, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxSize( -1,20 ), wxSL_HORIZONTAL );
	fgSizerVolume->Add( m_slider_se, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrl_se = new wxTextCtrl( this, wxID_ANY, wxT("255"), wxDefaultPosition, wxSize( 32,-1 ), 0 );
	fgSizerVolume->Add( m_textCtrl_se, 0, wxALL, 2 );
	
	m_staticText_voice = new wxStaticText( this, wxID_ANY, wxT("Voice"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_voice->Wrap( -1 );
	fgSizerVolume->Add( m_staticText_voice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_checkBox_voice = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_voice->SetValue(true); 
	fgSizerVolume->Add( m_checkBox_voice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_slider_voice = new wxSlider( this, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxSize( -1,20 ), wxSL_HORIZONTAL );
	fgSizerVolume->Add( m_slider_voice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrl_voice = new wxTextCtrl( this, wxID_ANY, wxT("255"), wxDefaultPosition, wxSize( 32,-1 ), 0 );
	fgSizerVolume->Add( m_textCtrl_voice, 0, wxALL, 2 );
	
	sbSizerSound->Add( fgSizerVolume, 1, wxEXPAND, 5 );
	
	m_checkBox_play_background = new wxCheckBox( this, wxID_ANY, wxT("バックグラウンド時に再生する"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_play_background->SetValue(true); 
	sbSizerSound->Add( m_checkBox_play_background, 0, wxALL, 2 );
	
	fgSizerLeft->Add( sbSizerSound, 1, wxALL|wxEXPAND, 2 );
	
	wxStaticBoxSizer* sbSizerEtc;
	sbSizerEtc = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("その他") ), wxVERTICAL );
	
	wxGridSizer* gSizer4;
	gSizer4 = new wxGridSizer( 4, 1, 0, 0 );
	
	m_checkBox_display_fps = new wxCheckBox( this, wxID_ANY, wxT("FPS表示"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer4->Add( m_checkBox_display_fps, 0, wxALL, 2 );
	
	m_checkBox_disable_movie = new wxCheckBox( this, wxID_ANY, wxT("ムービー無効"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer4->Add( m_checkBox_disable_movie, 0, wxALL, 2 );
	
	m_checkBox_disable_screen_saver = new wxCheckBox( this, wxID_ANY, wxT("スクリーンセーバー無効"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_disable_screen_saver->SetValue(true); 
	gSizer4->Add( m_checkBox_disable_screen_saver, 0, wxALL, 2 );
	
	m_checkBox_input_background = new wxCheckBox( this, wxID_ANY, wxT("バックグラウンド時に入力受付"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer4->Add( m_checkBox_input_background, 0, wxALL, 2 );
	
	m_checkBox_unlock_all_contents = new wxCheckBox( this, wxID_ANY, wxT("全ての隠し要素を開放"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer4->Add( m_checkBox_unlock_all_contents, 0, wxALL, 2 );
	
	sbSizerEtc->Add( gSizer4, 1, wxEXPAND, 5 );
	
	fgSizerLeft->Add( sbSizerEtc, 1, wxALL|wxEXPAND, 2 );
	
	fgSizerRoot->Add( fgSizerLeft, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerRight;
	fgSizerRight = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizerRight->SetFlexibleDirection( wxBOTH );
	fgSizerRight->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerControl1P;
	sbSizerControl1P = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("コントロール1P") ), wxVERTICAL );
	
	wxBoxSizer* bSizerDevice1P;
	bSizerDevice1P = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText81 = new wxStaticText( this, wxID_ANY, wxT("デバイス"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_staticText81->Wrap( -1 );
	bSizerDevice1P->Add( m_staticText81, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	wxArrayString m_choice_device1PChoices;
	m_choice_device1P = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), m_choice_device1PChoices, 0 );
	m_choice_device1P->SetSelection( 0 );
	bSizerDevice1P->Add( m_choice_device1P, 1, wxALL|wxEXPAND, 2 );
	
	sbSizerControl1P->Add( bSizerDevice1P, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerAnalogSense1P;
	bSizerAnalogSense1P = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText64 = new wxStaticText( this, wxID_ANY, wxT("アナログ感度"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText64->Wrap( -1 );
	bSizerAnalogSense1P->Add( m_staticText64, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_spin_analog_sense_1P = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 100, 0 );
	bSizerAnalogSense1P->Add( m_spin_analog_sense_1P, 0, wxALL, 5 );
	
	sbSizerControl1P->Add( bSizerAnalogSense1P, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerKeyMapping1P;
	fgSizerKeyMapping1P = new wxFlexGridSizer( 1, 4, 0, 0 );
	fgSizerKeyMapping1P->SetFlexibleDirection( wxBOTH );
	fgSizerKeyMapping1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizerUp_1P;
	fgSizerUp_1P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerUp_1P->SetFlexibleDirection( wxBOTH );
	fgSizerUp_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText18 = new wxStaticText( this, wxID_ANY, wxT("↑"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText18->Wrap( -1 );
	fgSizerUp_1P->Add( m_staticText18, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlUp_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerUp_1P->Add( m_textCtrlUp_1P, 0, wxALL, 2 );
	
	fgSizer11->Add( fgSizerUp_1P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerDown_1P;
	fgSizerDown_1P = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizerDown_1P->SetFlexibleDirection( wxBOTH );
	fgSizerDown_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText182 = new wxStaticText( this, wxID_ANY, wxT("↓"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText182->Wrap( -1 );
	fgSizerDown_1P->Add( m_staticText182, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlDown_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerDown_1P->Add( m_textCtrlDown_1P, 0, wxALL, 2 );
	
	fgSizer11->Add( fgSizerDown_1P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerLeft_1P;
	fgSizerLeft_1P = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizerLeft_1P->SetFlexibleDirection( wxBOTH );
	fgSizerLeft_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText183 = new wxStaticText( this, wxID_ANY, wxT("←"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText183->Wrap( -1 );
	fgSizerLeft_1P->Add( m_staticText183, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlLeft_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerLeft_1P->Add( m_textCtrlLeft_1P, 0, wxALL, 2 );
	
	fgSizer11->Add( fgSizerLeft_1P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerRight_1P;
	fgSizerRight_1P = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizerRight_1P->SetFlexibleDirection( wxBOTH );
	fgSizerRight_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText184 = new wxStaticText( this, wxID_ANY, wxT("→"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText184->Wrap( -1 );
	fgSizerRight_1P->Add( m_staticText184, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlRight_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerRight_1P->Add( m_textCtrlRight_1P, 0, wxALL, 2 );
	
	fgSizer11->Add( fgSizerRight_1P, 1, wxEXPAND, 5 );
	
	fgSizerKeyMapping1P->Add( fgSizer11, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer12;
	fgSizer12 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer12->SetFlexibleDirection( wxBOTH );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizerCircle_1P;
	fgSizerCircle_1P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerCircle_1P->SetFlexibleDirection( wxBOTH );
	fgSizerCircle_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText185 = new wxStaticText( this, wxID_ANY, wxT("○"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText185->Wrap( -1 );
	fgSizerCircle_1P->Add( m_staticText185, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlCircle_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerCircle_1P->Add( m_textCtrlCircle_1P, 0, wxALL, 2 );
	
	fgSizer12->Add( fgSizerCircle_1P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerCross_1P;
	fgSizerCross_1P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerCross_1P->SetFlexibleDirection( wxBOTH );
	fgSizerCross_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1851 = new wxStaticText( this, wxID_ANY, wxT("×"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText1851->Wrap( -1 );
	fgSizerCross_1P->Add( m_staticText1851, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlCross_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerCross_1P->Add( m_textCtrlCross_1P, 0, wxALL, 2 );
	
	fgSizer12->Add( fgSizerCross_1P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerTriangle_1P;
	fgSizerTriangle_1P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerTriangle_1P->SetFlexibleDirection( wxBOTH );
	fgSizerTriangle_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1852 = new wxStaticText( this, wxID_ANY, wxT("△"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText1852->Wrap( -1 );
	fgSizerTriangle_1P->Add( m_staticText1852, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlTriangle_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerTriangle_1P->Add( m_textCtrlTriangle_1P, 0, wxALL, 2 );
	
	fgSizer12->Add( fgSizerTriangle_1P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerSquare_1P;
	fgSizerSquare_1P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerSquare_1P->SetFlexibleDirection( wxBOTH );
	fgSizerSquare_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1853 = new wxStaticText( this, wxID_ANY, wxT("□"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText1853->Wrap( -1 );
	fgSizerSquare_1P->Add( m_staticText1853, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlSquare_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerSquare_1P->Add( m_textCtrlSquare_1P, 0, wxALL, 2 );
	
	fgSizer12->Add( fgSizerSquare_1P, 1, wxEXPAND, 5 );
	
	fgSizerKeyMapping1P->Add( fgSizer12, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer13;
	fgSizer13 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer13->SetFlexibleDirection( wxBOTH );
	fgSizer13->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizerL1_1P;
	fgSizerL1_1P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerL1_1P->SetFlexibleDirection( wxBOTH );
	fgSizerL1_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1854 = new wxStaticText( this, wxID_ANY, wxT("L1"), wxDefaultPosition, wxSize( 14,-1 ), 0 );
	m_staticText1854->Wrap( -1 );
	fgSizerL1_1P->Add( m_staticText1854, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlL1_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerL1_1P->Add( m_textCtrlL1_1P, 0, wxALL, 2 );
	
	fgSizer13->Add( fgSizerL1_1P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerR1_1P;
	fgSizerR1_1P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerR1_1P->SetFlexibleDirection( wxBOTH );
	fgSizerR1_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1856 = new wxStaticText( this, wxID_ANY, wxT("R1"), wxDefaultPosition, wxSize( 14,-1 ), 0 );
	m_staticText1856->Wrap( -1 );
	fgSizerR1_1P->Add( m_staticText1856, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlR1_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerR1_1P->Add( m_textCtrlR1_1P, 0, wxALL, 2 );
	
	fgSizer13->Add( fgSizerR1_1P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerL2_1P;
	fgSizerL2_1P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerL2_1P->SetFlexibleDirection( wxBOTH );
	fgSizerL2_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1855 = new wxStaticText( this, wxID_ANY, wxT("L2"), wxDefaultPosition, wxSize( 14,-1 ), 0 );
	m_staticText1855->Wrap( -1 );
	fgSizerL2_1P->Add( m_staticText1855, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlL2_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerL2_1P->Add( m_textCtrlL2_1P, 0, wxALL, 2 );
	
	fgSizer13->Add( fgSizerL2_1P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerR2_1P;
	fgSizerR2_1P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerR2_1P->SetFlexibleDirection( wxBOTH );
	fgSizerR2_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1857 = new wxStaticText( this, wxID_ANY, wxT("R2"), wxDefaultPosition, wxSize( 14,-1 ), 0 );
	m_staticText1857->Wrap( -1 );
	fgSizerR2_1P->Add( m_staticText1857, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlR2_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerR2_1P->Add( m_textCtrlR2_1P, 0, wxALL, 2 );
	
	fgSizer13->Add( fgSizerR2_1P, 1, wxEXPAND, 5 );
	
	fgSizerKeyMapping1P->Add( fgSizer13, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer14;
	fgSizer14 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer14->SetFlexibleDirection( wxBOTH );
	fgSizer14->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizerL3_1P;
	fgSizerL3_1P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerL3_1P->SetFlexibleDirection( wxBOTH );
	fgSizerL3_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText18541 = new wxStaticText( this, wxID_ANY, wxT("L3"), wxDefaultPosition, wxSize( 34,-1 ), 0 );
	m_staticText18541->Wrap( -1 );
	fgSizerL3_1P->Add( m_staticText18541, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlL3_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerL3_1P->Add( m_textCtrlL3_1P, 0, wxALL, 2 );
	
	fgSizer14->Add( fgSizerL3_1P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerR3_1P;
	fgSizerR3_1P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerR3_1P->SetFlexibleDirection( wxBOTH );
	fgSizerR3_1P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText18542 = new wxStaticText( this, wxID_ANY, wxT("R3"), wxDefaultPosition, wxSize( 34,-1 ), 0 );
	m_staticText18542->Wrap( -1 );
	fgSizerR3_1P->Add( m_staticText18542, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlR3_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerR3_1P->Add( m_textCtrlR3_1P, 0, wxALL, 2 );
	
	fgSizer14->Add( fgSizerR3_1P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerStart_1P1;
	fgSizerStart_1P1 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerStart_1P1->SetFlexibleDirection( wxBOTH );
	fgSizerStart_1P1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText185412 = new wxStaticText( this, wxID_ANY, wxT("Start"), wxDefaultPosition, wxSize( 34,-1 ), 0 );
	m_staticText185412->Wrap( -1 );
	fgSizerStart_1P1->Add( m_staticText185412, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlStart_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerStart_1P1->Add( m_textCtrlStart_1P, 0, wxALL, 2 );
	
	fgSizer14->Add( fgSizerStart_1P1, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerStart_1P2;
	fgSizerStart_1P2 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerStart_1P2->SetFlexibleDirection( wxBOTH );
	fgSizerStart_1P2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText185413 = new wxStaticText( this, wxID_ANY, wxT("Select"), wxDefaultPosition, wxSize( 34,-1 ), 0 );
	m_staticText185413->Wrap( -1 );
	fgSizerStart_1P2->Add( m_staticText185413, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlSelect_1P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerStart_1P2->Add( m_textCtrlSelect_1P, 0, wxALL, 2 );
	
	fgSizer14->Add( fgSizerStart_1P2, 1, wxEXPAND, 5 );
	
	fgSizerKeyMapping1P->Add( fgSizer14, 1, wxEXPAND, 5 );
	
	sbSizerControl1P->Add( fgSizerKeyMapping1P, 1, wxEXPAND, 5 );
	
	fgSizerRight->Add( sbSizerControl1P, 1, wxALL|wxEXPAND, 2 );
	
	wxStaticBoxSizer* sbSizerControl2P;
	sbSizerControl2P = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("コントロール2P") ), wxVERTICAL );
	
	wxBoxSizer* bSizerDevice2P;
	bSizerDevice2P = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText811 = new wxStaticText( this, wxID_ANY, wxT("デバイス"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_staticText811->Wrap( -1 );
	bSizerDevice2P->Add( m_staticText811, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	wxArrayString m_choice_device2PChoices;
	m_choice_device2P = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), m_choice_device2PChoices, 0 );
	m_choice_device2P->SetSelection( 0 );
	bSizerDevice2P->Add( m_choice_device2P, 1, wxALL|wxEXPAND, 2 );
	
	sbSizerControl2P->Add( bSizerDevice2P, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerAnalogSense2P;
	bSizerAnalogSense2P = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText641 = new wxStaticText( this, wxID_ANY, wxT("アナログ感度"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText641->Wrap( -1 );
	bSizerAnalogSense2P->Add( m_staticText641, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_spin_analog_sense_2P = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 100, 0 );
	bSizerAnalogSense2P->Add( m_spin_analog_sense_2P, 0, wxALL, 5 );
	
	sbSizerControl2P->Add( bSizerAnalogSense2P, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerKeyMapping2P;
	fgSizerKeyMapping2P = new wxFlexGridSizer( 1, 4, 0, 0 );
	fgSizerKeyMapping2P->SetFlexibleDirection( wxBOTH );
	fgSizerKeyMapping2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer21;
	fgSizer21 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer21->SetFlexibleDirection( wxBOTH );
	fgSizer21->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizerUp_2P;
	fgSizerUp_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerUp_2P->SetFlexibleDirection( wxBOTH );
	fgSizerUp_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText181 = new wxStaticText( this, wxID_ANY, wxT("↑"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText181->Wrap( -1 );
	fgSizerUp_2P->Add( m_staticText181, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlUp_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerUp_2P->Add( m_textCtrlUp_2P, 0, wxALL, 2 );
	
	fgSizer21->Add( fgSizerUp_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerDown_2P;
	fgSizerDown_2P = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizerDown_2P->SetFlexibleDirection( wxBOTH );
	fgSizerDown_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1821 = new wxStaticText( this, wxID_ANY, wxT("↓"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText1821->Wrap( -1 );
	fgSizerDown_2P->Add( m_staticText1821, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlDown_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerDown_2P->Add( m_textCtrlDown_2P, 0, wxALL, 2 );
	
	fgSizer21->Add( fgSizerDown_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerLeft_2P;
	fgSizerLeft_2P = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizerLeft_2P->SetFlexibleDirection( wxBOTH );
	fgSizerLeft_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1831 = new wxStaticText( this, wxID_ANY, wxT("←"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText1831->Wrap( -1 );
	fgSizerLeft_2P->Add( m_staticText1831, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlLeft_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerLeft_2P->Add( m_textCtrlLeft_2P, 0, wxALL, 2 );
	
	fgSizer21->Add( fgSizerLeft_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerRight_2P;
	fgSizerRight_2P = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizerRight_2P->SetFlexibleDirection( wxBOTH );
	fgSizerRight_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1841 = new wxStaticText( this, wxID_ANY, wxT("→"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText1841->Wrap( -1 );
	fgSizerRight_2P->Add( m_staticText1841, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlRight_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerRight_2P->Add( m_textCtrlRight_2P, 0, wxALL, 2 );
	
	fgSizer21->Add( fgSizerRight_2P, 1, wxEXPAND, 5 );
	
	fgSizerKeyMapping2P->Add( fgSizer21, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer22;
	fgSizer22 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer22->SetFlexibleDirection( wxBOTH );
	fgSizer22->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizerCircle_2P;
	fgSizerCircle_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerCircle_2P->SetFlexibleDirection( wxBOTH );
	fgSizerCircle_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1858 = new wxStaticText( this, wxID_ANY, wxT("○"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText1858->Wrap( -1 );
	fgSizerCircle_2P->Add( m_staticText1858, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlCircle_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerCircle_2P->Add( m_textCtrlCircle_2P, 0, wxALL, 2 );
	
	fgSizer22->Add( fgSizerCircle_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerCross_2P;
	fgSizerCross_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerCross_2P->SetFlexibleDirection( wxBOTH );
	fgSizerCross_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText18511 = new wxStaticText( this, wxID_ANY, wxT("×"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText18511->Wrap( -1 );
	fgSizerCross_2P->Add( m_staticText18511, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlCross_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerCross_2P->Add( m_textCtrlCross_2P, 0, wxALL, 2 );
	
	fgSizer22->Add( fgSizerCross_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerTriangle_2P;
	fgSizerTriangle_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerTriangle_2P->SetFlexibleDirection( wxBOTH );
	fgSizerTriangle_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText18521 = new wxStaticText( this, wxID_ANY, wxT("△"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText18521->Wrap( -1 );
	fgSizerTriangle_2P->Add( m_staticText18521, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlTriangle_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerTriangle_2P->Add( m_textCtrlTriangle_2P, 0, wxALL, 2 );
	
	fgSizer22->Add( fgSizerTriangle_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerSquare_2P;
	fgSizerSquare_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerSquare_2P->SetFlexibleDirection( wxBOTH );
	fgSizerSquare_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText18531 = new wxStaticText( this, wxID_ANY, wxT("□"), wxDefaultPosition, wxSize( 12,-1 ), 0 );
	m_staticText18531->Wrap( -1 );
	fgSizerSquare_2P->Add( m_staticText18531, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlSquare_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerSquare_2P->Add( m_textCtrlSquare_2P, 0, wxALL, 2 );
	
	fgSizer22->Add( fgSizerSquare_2P, 1, wxEXPAND, 5 );
	
	fgSizerKeyMapping2P->Add( fgSizer22, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer23;
	fgSizer23 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer23->SetFlexibleDirection( wxBOTH );
	fgSizer23->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizerL1_2P;
	fgSizerL1_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerL1_2P->SetFlexibleDirection( wxBOTH );
	fgSizerL1_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText18543 = new wxStaticText( this, wxID_ANY, wxT("L1"), wxDefaultPosition, wxSize( 14,-1 ), 0 );
	m_staticText18543->Wrap( -1 );
	fgSizerL1_2P->Add( m_staticText18543, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlL1_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerL1_2P->Add( m_textCtrlL1_2P, 0, wxALL, 2 );
	
	fgSizer23->Add( fgSizerL1_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerR1_2P;
	fgSizerR1_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerR1_2P->SetFlexibleDirection( wxBOTH );
	fgSizerR1_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText18561 = new wxStaticText( this, wxID_ANY, wxT("R1"), wxDefaultPosition, wxSize( 14,-1 ), 0 );
	m_staticText18561->Wrap( -1 );
	fgSizerR1_2P->Add( m_staticText18561, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlR1_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerR1_2P->Add( m_textCtrlR1_2P, 0, wxALL, 2 );
	
	fgSizer23->Add( fgSizerR1_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerL2_2P;
	fgSizerL2_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerL2_2P->SetFlexibleDirection( wxBOTH );
	fgSizerL2_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText18551 = new wxStaticText( this, wxID_ANY, wxT("L2"), wxDefaultPosition, wxSize( 14,-1 ), 0 );
	m_staticText18551->Wrap( -1 );
	fgSizerL2_2P->Add( m_staticText18551, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlL2_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerL2_2P->Add( m_textCtrlL2_2P, 0, wxALL, 2 );
	
	fgSizer23->Add( fgSizerL2_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerR2_2P;
	fgSizerR2_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerR2_2P->SetFlexibleDirection( wxBOTH );
	fgSizerR2_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText18571 = new wxStaticText( this, wxID_ANY, wxT("R2"), wxDefaultPosition, wxSize( 14,-1 ), 0 );
	m_staticText18571->Wrap( -1 );
	fgSizerR2_2P->Add( m_staticText18571, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlR2_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerR2_2P->Add( m_textCtrlR2_2P, 0, wxALL, 2 );
	
	fgSizer23->Add( fgSizerR2_2P, 1, wxEXPAND, 5 );
	
	fgSizerKeyMapping2P->Add( fgSizer23, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer24;
	fgSizer24 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer24->SetFlexibleDirection( wxBOTH );
	fgSizer24->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizerL3_2P;
	fgSizerL3_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerL3_2P->SetFlexibleDirection( wxBOTH );
	fgSizerL3_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText185414 = new wxStaticText( this, wxID_ANY, wxT("L3"), wxDefaultPosition, wxSize( 34,-1 ), 0 );
	m_staticText185414->Wrap( -1 );
	fgSizerL3_2P->Add( m_staticText185414, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlL3_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerL3_2P->Add( m_textCtrlL3_2P, 0, wxALL, 2 );
	
	fgSizer24->Add( fgSizerL3_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerR3_2P;
	fgSizerR3_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerR3_2P->SetFlexibleDirection( wxBOTH );
	fgSizerR3_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1854141 = new wxStaticText( this, wxID_ANY, wxT("R3"), wxDefaultPosition, wxSize( 34,-1 ), 0 );
	m_staticText1854141->Wrap( -1 );
	fgSizerR3_2P->Add( m_staticText1854141, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlR3_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerR3_2P->Add( m_textCtrlR3_2P, 0, wxALL, 2 );
	
	fgSizer24->Add( fgSizerR3_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerStart_2P;
	fgSizerStart_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerStart_2P->SetFlexibleDirection( wxBOTH );
	fgSizerStart_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText185411 = new wxStaticText( this, wxID_ANY, wxT("Start"), wxDefaultPosition, wxSize( 34,-1 ), 0 );
	m_staticText185411->Wrap( -1 );
	fgSizerStart_2P->Add( m_staticText185411, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlStart_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerStart_2P->Add( m_textCtrlStart_2P, 0, wxALL, 2 );
	
	fgSizer24->Add( fgSizerStart_2P, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizerSelect_2P;
	fgSizerSelect_2P = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizerSelect_2P->SetFlexibleDirection( wxBOTH );
	fgSizerSelect_2P->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText185421 = new wxStaticText( this, wxID_ANY, wxT("Select"), wxDefaultPosition, wxSize( 34,-1 ), 0 );
	m_staticText185421->Wrap( -1 );
	fgSizerSelect_2P->Add( m_staticText185421, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlSelect_2P = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 56,-1 ), wxTE_READONLY );
	fgSizerSelect_2P->Add( m_textCtrlSelect_2P, 0, wxALL, 2 );
	
	fgSizer24->Add( fgSizerSelect_2P, 1, wxEXPAND, 2 );
	
	fgSizerKeyMapping2P->Add( fgSizer24, 1, wxEXPAND, 5 );
	
	sbSizerControl2P->Add( fgSizerKeyMapping2P, 1, wxEXPAND, 5 );
	
	fgSizerRight->Add( sbSizerControl2P, 1, wxALL|wxEXPAND, 2 );
	
	m_textCtrl_dummy = new wxTextCtrl( this, wxID_ANY, wxT("for receive input"), wxDefaultPosition, wxDefaultSize, 0|wxWANTS_CHARS );
	m_textCtrl_dummy->Hide();
	
	fgSizerRight->Add( m_textCtrl_dummy, 0, wxALL, 5 );
	
	fgSizerRoot->Add( fgSizerRight, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizerRoot );
	this->Layout();
	fgSizerRoot->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( GeneralSheet::on_init ) );
	this->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ) );
	m_staticText11->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_choice_display_device->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText111->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_choice_display_mode->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GeneralSheet::onchoice_screen_mode ), NULL, this );
	m_choice_display_mode->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_choice_color_depth->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_wait_vsync->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_frame_skip->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_keep_aspect_ratio->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText_bgm->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_bgm->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_bgm->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_bgm->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_textCtrl_bgm->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrl_bgm->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GeneralSheet::on_text_volume ), NULL, this );
	m_staticText_se->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_se->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_se->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_se->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_textCtrl_se->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrl_se->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GeneralSheet::on_text_volume ), NULL, this );
	m_staticText_voice->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_voice->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_voice->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_voice->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_textCtrl_voice->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrl_voice->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GeneralSheet::on_text_volume ), NULL, this );
	m_checkBox_play_background->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_display_fps->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_disable_movie->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_disable_screen_saver->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_input_background->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_unlock_all_contents->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText81->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_choice_device1P->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GeneralSheet::on_choice_device ), NULL, this );
	m_choice_device1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText64->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_spin_analog_sense_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlUp_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText182->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlDown_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText183->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlLeft_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText184->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlRight_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlCircle_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1851->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlCross_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1852->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlTriangle_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1853->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlSquare_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1854->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL1_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1856->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR1_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1855->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL2_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1857->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR2_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18541->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL3_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18542->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR3_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185412->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlStart_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185413->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlSelect_1P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText811->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_choice_device2P->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GeneralSheet::on_choice_device ), NULL, this );
	m_choice_device2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText641->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_spin_analog_sense_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText181->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlUp_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1821->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlDown_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1831->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlLeft_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1841->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlRight_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1858->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlCircle_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18511->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlCross_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18521->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlTriangle_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18531->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlSquare_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18543->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL1_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18561->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR1_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18551->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL2_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18571->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR2_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185414->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL3_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1854141->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR3_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185411->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlStart_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185421->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlSelect_2P->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
}

GeneralSheet::~GeneralSheet()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( GeneralSheet::on_init ) );
	this->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ) );
	m_staticText11->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_choice_display_device->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText111->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_choice_display_mode->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GeneralSheet::onchoice_screen_mode ), NULL, this );
	m_choice_display_mode->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_choice_color_depth->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_wait_vsync->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_frame_skip->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_keep_aspect_ratio->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText_bgm->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_bgm->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_bgm->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_bgm->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_bgm->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_textCtrl_bgm->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrl_bgm->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GeneralSheet::on_text_volume ), NULL, this );
	m_staticText_se->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_se->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_se->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_se->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_se->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_textCtrl_se->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrl_se->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GeneralSheet::on_text_volume ), NULL, this );
	m_staticText_voice->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_voice->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_voice->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_slider_voice->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_slider_voice->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GeneralSheet::on_scroll_volume ), NULL, this );
	m_textCtrl_voice->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrl_voice->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GeneralSheet::on_text_volume ), NULL, this );
	m_checkBox_play_background->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_display_fps->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_disable_movie->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_disable_screen_saver->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_input_background->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_checkBox_unlock_all_contents->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText81->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_choice_device1P->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GeneralSheet::on_choice_device ), NULL, this );
	m_choice_device1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText64->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_spin_analog_sense_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlUp_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText182->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlDown_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText183->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlLeft_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText184->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlRight_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlCircle_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1851->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlCross_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1852->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlTriangle_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1853->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlSquare_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1854->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL1_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1856->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR1_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1855->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL2_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1857->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR2_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18541->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL3_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18542->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR3_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185412->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlStart_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185413->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlSelect_1P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText811->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_choice_device2P->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GeneralSheet::on_choice_device ), NULL, this );
	m_choice_device2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText641->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_spin_analog_sense_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText181->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlUp_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1821->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlDown_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1831->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlLeft_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1841->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlRight_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1858->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlCircle_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18511->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlCross_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18521->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlTriangle_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18531->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlSquare_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18543->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL1_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18561->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR1_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18551->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL2_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText18571->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR2_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185414->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlL3_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText1854141->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlR3_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185411->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlStart_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_staticText185421->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	m_textCtrlSelect_2P->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GeneralSheet::on_left_down ), NULL, this );
	
}

NetworkSheet::NetworkSheet( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizerConnection;
	sbSizerConnection = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("接続設定") ), wxVERTICAL );
	
	wxBoxSizer* bSizerPort;
	bSizerPort = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText_port = new wxStaticText( this, wxID_ANY, wxT("ポート番号(UDP)"), wxDefaultPosition, wxSize( 96,-1 ), 0 );
	m_staticText_port->Wrap( -1 );
	bSizerPort->Add( m_staticText_port, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrl_port = new wxTextCtrl( this, wxID_ANY, wxT("12345"), wxDefaultPosition, wxSize( 48,-1 ), 0 );
	m_textCtrl_port->SetToolTip( wxT("通信に使用するポート番号を入力します") );
	
	bSizerPort->Add( m_textCtrl_port, 0, wxALL, 2 );
	
	sbSizerConnection->Add( bSizerPort, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerDelayFrame;
	bSizerDelayFrame = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText_delay = new wxStaticText( this, wxID_ANY, wxT("ディレイフレーム"), wxDefaultPosition, wxSize( 96,-1 ), 0 );
	m_staticText_delay->Wrap( -1 );
	bSizerDelayFrame->Add( m_staticText_delay, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	wxString m_choice_delayChoices[] = { wxT("0"), wxT("1"), wxT("2"), wxT("3"), wxT("4") };
	int m_choice_delayNChoices = sizeof( m_choice_delayChoices ) / sizeof( wxString );
	m_choice_delay = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 48,-1 ), m_choice_delayNChoices, m_choice_delayChoices, 0 );
	m_choice_delay->SetSelection( 0 );
	m_choice_delay->SetToolTip( wxT("入力遅延をフレーム単位で入力します\nオフライン時も適用されます") );
	
	bSizerDelayFrame->Add( m_choice_delay, 0, wxALL, 2 );
	
	sbSizerConnection->Add( bSizerDelayFrame, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerRollbackFrame;
	bSizerRollbackFrame = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText_rollback = new wxStaticText( this, wxID_ANY, wxT("ロールバックフレーム"), wxDefaultPosition, wxSize( 96,-1 ), 0 );
	m_staticText_rollback->Wrap( -1 );
	bSizerRollbackFrame->Add( m_staticText_rollback, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	wxString m_choice_rollbackChoices[] = { wxT("0"), wxT("1"), wxT("2"), wxT("3"), wxT("4") };
	int m_choice_rollbackNChoices = sizeof( m_choice_rollbackChoices ) / sizeof( wxString );
	m_choice_rollback = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 48,-1 ), m_choice_rollbackNChoices, m_choice_rollbackChoices, 0 );
	m_choice_rollback->SetSelection( 2 );
	m_choice_rollback->SetToolTip( wxT("ロールバック可能なフレーム数を入力します") );
	
	bSizerRollbackFrame->Add( m_choice_rollback, 0, wxALL, 2 );
	
	sbSizerConnection->Add( bSizerRollbackFrame, 0, wxEXPAND, 5 );
	
	fgSizer11->Add( sbSizerConnection, 1, wxALL|wxEXPAND, 2 );
	
	wxStaticBoxSizer* sbSizerRule;
	sbSizerRule = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("対戦条件") ), wxVERTICAL );
	
	m_checkBox_ex = new wxCheckBox( this, wxID_ANY, wxT("Exキャラ使用可能"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_ex->SetToolTip( wxT("Exキャラを使用する場合オンにします\n相手もオンの場合に限り、対戦にて使用できます。") );
	
	sbSizerRule->Add( m_checkBox_ex, 0, wxALL, 2 );
	
	m_checkBox_cheat = new wxCheckBox( this, wxID_ANY, wxT("金・黒キャラ使用可能"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_cheat->SetToolTip( wxT("金・黒キャラを使用する場合オンにします\n相手もオンの場合に限り、対戦にて使用できます。") );
	
	sbSizerRule->Add( m_checkBox_cheat, 0, wxALL, 2 );
	
	wxBoxSizer* bSizerRound;
	bSizerRound = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText_round = new wxStaticText( this, wxID_ANY, wxT("ラウンド数"), wxDefaultPosition, wxSize( 96,-1 ), 0 );
	m_staticText_round->Wrap( -1 );
	bSizerRound->Add( m_staticText_round, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	wxString m_choice_roundChoices[] = { wxT("3"), wxT("5") };
	int m_choice_roundNChoices = sizeof( m_choice_roundChoices ) / sizeof( wxString );
	m_choice_round = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 48,-1 ), m_choice_roundNChoices, m_choice_roundChoices, 0 );
	m_choice_round->SetSelection( 1 );
	m_choice_round->SetToolTip( wxT("ラウンド数を指定します\n相手と設定が異なる場合、接続された方の設定が適用されます") );
	
	bSizerRound->Add( m_choice_round, 0, wxALL, 2 );
	
	sbSizerRule->Add( bSizerRound, 0, wxEXPAND, 5 );
	
	fgSizer11->Add( sbSizerRule, 1, wxALL|wxEXPAND, 2 );
	
	wxStaticBoxSizer* sbSizerWatch;
	sbSizerWatch = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("観戦設定") ), wxVERTICAL );
	
	m_checkBox_allow_watch = new wxCheckBox( this, wxID_ANY, wxT("観戦を許可する"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_allow_watch->SetValue(true); 
	m_checkBox_allow_watch->SetToolTip( wxT("他ユーザーに観戦を許可します\n相手も観戦が許可されている場合に観戦が可能です") );
	
	sbSizerWatch->Add( m_checkBox_allow_watch, 0, wxALL, 2 );
	
	m_checkBox_allow_intrusion = new wxCheckBox( this, wxID_ANY, wxT("観戦中のを乱入を許可する"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerWatch->Add( m_checkBox_allow_intrusion, 0, wxALL, 2 );
	
	m_checkBox_save_replay = new wxCheckBox( this, wxID_ANY, wxT("観戦したリプレイを保存する"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerWatch->Add( m_checkBox_save_replay, 0, wxALL, 2 );
	
	wxBoxSizer* bSizerMaxRelay;
	bSizerMaxRelay = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText_max_relay = new wxStaticText( this, wxID_ANY, wxT("中継可能ノード数"), wxDefaultPosition, wxSize( 96,-1 ), 0 );
	m_staticText_max_relay->Wrap( -1 );
	bSizerMaxRelay->Add( m_staticText_max_relay, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	wxString m_choice_max_relayChoices[] = { wxT("0"), wxT("1"), wxT("2"), wxT("3") };
	int m_choice_max_relayNChoices = sizeof( m_choice_max_relayChoices ) / sizeof( wxString );
	m_choice_max_relay = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 48,-1 ), m_choice_max_relayNChoices, m_choice_max_relayChoices, 0 );
	m_choice_max_relay->SetSelection( 1 );
	m_choice_max_relay->SetToolTip( wxT("対戦・観戦時のリプレイデータを中継可能なノード数を指定します") );
	
	bSizerMaxRelay->Add( m_choice_max_relay, 0, wxALL, 2 );
	
	sbSizerWatch->Add( bSizerMaxRelay, 0, wxEXPAND, 5 );
	
	fgSizer11->Add( sbSizerWatch, 1, wxEXPAND, 5 );
	
	bSizer1->Add( fgSizer11, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizerLobby;
	sbSizerLobby = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("ロビー") ), wxVERTICAL );
	
	wxBoxSizer* bSizerLobbyURL;
	bSizerLobbyURL = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText2031 = new wxStaticText( this, wxID_ANY, wxT("URL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2031->Wrap( -1 );
	bSizerLobbyURL->Add( m_staticText2031, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_comboBox_lobby_address = new wxComboBox( this, wxID_ANY, wxT("localhost/g-emu-lobby.cgi"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	m_comboBox_lobby_address->SetToolTip( wxT("ロビースクリプトのURLを選択します") );
	
	bSizerLobbyURL->Add( m_comboBox_lobby_address, 1, wxALL|wxEXPAND, 2 );
	
	m_button_lobby_address_add = new wxButton( this, wxID_ANY, wxT("追加"), wxDefaultPosition, wxSize( 32,-1 ), 0 );
	bSizerLobbyURL->Add( m_button_lobby_address_add, 0, wxALL, 2 );
	
	m_button_lobby_address_del = new wxButton( this, wxID_ANY, wxT("削除"), wxDefaultPosition, wxSize( 32,-1 ), 0 );
	bSizerLobbyURL->Add( m_button_lobby_address_del, 0, wxALL, 2 );
	
	sbSizerLobby->Add( bSizerLobbyURL, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerLobbyFont;
	bSizerLobbyFont = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText1992 = new wxStaticText( this, wxID_ANY, wxT("フォント"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1992->Wrap( -1 );
	bSizerLobbyFont->Add( m_staticText1992, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_comboBox_font_face = new wxComboBox( this, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
	m_comboBox_font_face->SetToolTip( wxT("ロビーのフォントを指定します") );
	
	bSizerLobbyFont->Add( m_comboBox_font_face, 1, wxALL|wxEXPAND, 2 );
	
	m_staticText19911 = new wxStaticText( this, wxID_ANY, wxT("サイズ"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText19911->Wrap( -1 );
	bSizerLobbyFont->Add( m_staticText19911, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_textCtrl_font_size = new wxTextCtrl( this, wxID_ANY, wxT("12"), wxDefaultPosition, wxSize( 32,-1 ), 0 );
	m_textCtrl_font_size->SetToolTip( wxT("ロビーのフォントサイズを指定します") );
	
	bSizerLobbyFont->Add( m_textCtrl_font_size, 0, wxALL, 2 );
	
	sbSizerLobby->Add( bSizerLobbyFont, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxVERTICAL );
	
	m_checkBox_antialias = new wxCheckBox( this, wxID_ANY, wxT("フォントをアンチエイリアスで表示する"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_antialias->SetToolTip( wxT("ロビーのフォントにアンチエイリアスを適用します") );
	
	bSizer31->Add( m_checkBox_antialias, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	sbSizerLobby->Add( bSizer31, 1, wxEXPAND, 5 );
	
	bSizer12->Add( sbSizerLobby, 0, wxALL|wxEXPAND, 2 );
	
	wxStaticBoxSizer* sbSizerDeny;
	sbSizerDeny = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("拒否設定") ), wxVERTICAL );
	
	wxBoxSizer* bSizerDenyType;
	bSizerDenyType = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText86 = new wxStaticText( this, wxID_ANY, wxT("特定プレイヤーの拒否"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_staticText86->Wrap( -1 );
	bSizerDenyType->Add( m_staticText86, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	wxString m_choice_deny_typeChoices[] = { wxT("IPアドレス"), wxT("クライアントコード") };
	int m_choice_deny_typeNChoices = sizeof( m_choice_deny_typeChoices ) / sizeof( wxString );
	m_choice_deny_type = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), m_choice_deny_typeNChoices, m_choice_deny_typeChoices, 0 );
	m_choice_deny_type->SetSelection( 1 );
	m_choice_deny_type->SetToolTip( wxT("指定プレイヤーの拒否に用いる情報を指定します\nクライアントコード　：PC固有の情報から算出した文字列\nIPアドレス　　　　　：対戦当時使用していたグローバルIPアドレス") );
	
	bSizerDenyType->Add( m_choice_deny_type, 0, wxALL, 2 );
	
	sbSizerDeny->Add( bSizerDenyType, 0, wxEXPAND, 5 );
	
	m_checkBox_deny_slow = new wxCheckBox( this, wxID_ANY, wxT("処理落ち率が一定以上のプレイヤーを拒否する"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_deny_slow->SetToolTip( wxT("処理落ち率の高いプレイヤーからの接続要求に応答しません\nこちらからの接続は可能です") );
	
	sbSizerDeny->Add( m_checkBox_deny_slow, 0, wxALL, 2 );
	
	m_checkBox_deny_disconnect = new wxCheckBox( this, wxID_ANY, wxT("切断率が30%以上のプレイヤーを拒否する"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_deny_disconnect->SetToolTip( wxT("切断率の高いプレイヤーからの接続要求に応答しません\nこちらからの接続は可能です") );
	
	sbSizerDeny->Add( m_checkBox_deny_disconnect, 0, wxALL, 2 );
	
	bSizer12->Add( sbSizerDeny, 0, wxALL, 2 );
	
	wxBoxSizer* bSizerProfile;
	bSizerProfile = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizerProfile;
	sbSizerProfile = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("プロファイル") ), wxVERTICAL );
	
	wxBoxSizer* bSizerPlayerName;
	bSizerPlayerName = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText_player_name = new wxStaticText( this, wxID_ANY, wxT("プレイヤー名"), wxDefaultPosition, wxSize( 64,-1 ), 0 );
	m_staticText_player_name->Wrap( -1 );
	bSizerPlayerName->Add( m_staticText_player_name, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrl_player_name = new wxTextCtrl( this, wxID_ANY, wxT("ああああああああああ"), wxDefaultPosition, wxSize( 160,-1 ), 0 );
	m_textCtrl_player_name->SetToolTip( wxT("プレイヤー名を入力します") );
	
	bSizerPlayerName->Add( m_textCtrl_player_name, 0, wxALL, 2 );
	
	sbSizerProfile->Add( bSizerPlayerName, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerTripKey;
	bSizerTripKey = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText_tripkey = new wxStaticText( this, wxID_ANY, wxT("トリップキー"), wxDefaultPosition, wxSize( 64,-1 ), 0 );
	m_staticText_tripkey->Wrap( -1 );
	bSizerTripKey->Add( m_staticText_tripkey, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrl_tripkey = new wxTextCtrl( this, wxID_ANY, wxT("12345"), wxDefaultPosition, wxSize( 160,-1 ), 0 );
	m_textCtrl_tripkey->SetToolTip( wxT("プレイヤー名の末尾にトリップ文字列を付加します\n騙りや名称重複を防止するために使用します\n不要であれば空白でも問題ありません") );
	
	bSizerTripKey->Add( m_textCtrl_tripkey, 0, wxALL, 2 );
	
	sbSizerProfile->Add( bSizerTripKey, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerUserData1;
	bSizerUserData1 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText_rank = new wxStaticText( this, wxID_ANY, wxT("ランク"), wxDefaultPosition, wxSize( 64,-1 ), 0 );
	m_staticText_rank->Wrap( -1 );
	bSizerUserData1->Add( m_staticText_rank, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrl_rank = new wxTextCtrl( this, wxID_ANY, wxT("12345"), wxDefaultPosition, wxSize( 44,-1 ), wxTE_READONLY );
	m_textCtrl_rank->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	m_textCtrl_rank->SetToolTip( wxT("他のプレイヤーとの相対的なランクを表します") );
	
	bSizerUserData1->Add( m_textCtrl_rank, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_staticText_win_rate = new wxStaticText( this, wxID_ANY, wxT("勝率"), wxDefaultPosition, wxSize( 64,-1 ), 0 );
	m_staticText_win_rate->Wrap( -1 );
	bSizerUserData1->Add( m_staticText_win_rate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_textCtrl_win_rate = new wxTextCtrl( this, wxID_ANY, wxT("12345"), wxDefaultPosition, wxSize( 44,-1 ), wxTE_READONLY );
	m_textCtrl_win_rate->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	m_textCtrl_win_rate->SetToolTip( wxT("通算勝率を表します\n総対戦数が50未満の場合、算出されません") );
	
	bSizerUserData1->Add( m_textCtrl_win_rate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	sbSizerProfile->Add( bSizerUserData1, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerUserData2;
	bSizerUserData2 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText_win_count = new wxStaticText( this, wxID_ANY, wxT("連勝数"), wxDefaultPosition, wxSize( 64,-1 ), 0 );
	m_staticText_win_count->Wrap( -1 );
	bSizerUserData2->Add( m_staticText_win_count, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrl_win_count = new wxTextCtrl( this, wxID_ANY, wxT("12345"), wxDefaultPosition, wxSize( 44,-1 ), wxTE_READONLY );
	m_textCtrl_win_count->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	m_textCtrl_win_count->SetToolTip( wxT("連勝回数を表します") );
	
	bSizerUserData2->Add( m_textCtrl_win_count, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_staticText_slow_rate = new wxStaticText( this, wxID_ANY, wxT("処理落ち率"), wxDefaultPosition, wxSize( 64,-1 ), 0 );
	m_staticText_slow_rate->Wrap( -1 );
	bSizerUserData2->Add( m_staticText_slow_rate, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrl_slow_rate = new wxTextCtrl( this, wxID_ANY, wxT("12345"), wxDefaultPosition, wxSize( 44,-1 ), wxTE_READONLY );
	m_textCtrl_slow_rate->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	m_textCtrl_slow_rate->SetToolTip( wxT("対戦時の処理落ち率を表します\n総対戦数が50未満の場合、算出されません") );
	
	bSizerUserData2->Add( m_textCtrl_slow_rate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	sbSizerProfile->Add( bSizerUserData2, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerUserData3;
	bSizerUserData3 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText_game_count = new wxStaticText( this, wxID_ANY, wxT("総対戦数"), wxDefaultPosition, wxSize( 64,-1 ), 0 );
	m_staticText_game_count->Wrap( -1 );
	bSizerUserData3->Add( m_staticText_game_count, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrl_game_count = new wxTextCtrl( this, wxID_ANY, wxT("12345"), wxDefaultPosition, wxSize( 44,-1 ), wxTE_READONLY );
	m_textCtrl_game_count->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	m_textCtrl_game_count->SetToolTip( wxT("通算対戦回数を表します") );
	
	bSizerUserData3->Add( m_textCtrl_game_count, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_staticText_disconnect_rate = new wxStaticText( this, wxID_ANY, wxT("切断率"), wxDefaultPosition, wxSize( 64,-1 ), 0 );
	m_staticText_disconnect_rate->Wrap( -1 );
	bSizerUserData3->Add( m_staticText_disconnect_rate, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrl_disconnect_rate = new wxTextCtrl( this, wxID_ANY, wxT("12345"), wxDefaultPosition, wxSize( 44,-1 ), wxTE_READONLY );
	m_textCtrl_disconnect_rate->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	m_textCtrl_disconnect_rate->SetToolTip( wxT("対戦時の切断率を表します\n総対戦数が50未満の場合、算出されません") );
	
	bSizerUserData3->Add( m_textCtrl_disconnect_rate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	sbSizerProfile->Add( bSizerUserData3, 1, wxEXPAND, 5 );
	
	m_button_edit_comment = new wxButton( this, wxID_ANY, wxT("コメント編集"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerProfile->Add( m_button_edit_comment, 0, wxALL|wxALIGN_RIGHT, 2 );
	
	bSizerProfile->Add( sbSizerProfile, 0, wxALL|wxEXPAND, 2 );
	
	bSizer12->Add( bSizerProfile, 0, wxEXPAND, 5 );
	
	bSizer1->Add( bSizer12, 1, wxEXPAND, 5 );
	
	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( NetworkSheet::on_init ) );
	m_button_lobby_address_add->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( NetworkSheet::on_button_clicked_lobby_address_add ), NULL, this );
	m_button_lobby_address_del->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( NetworkSheet::on_button_clicked_lobby_address_del ), NULL, this );
	m_button_edit_comment->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( NetworkSheet::on_button_clicked_edit_comment ), NULL, this );
}

NetworkSheet::~NetworkSheet()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( NetworkSheet::on_init ) );
	m_button_lobby_address_add->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( NetworkSheet::on_button_clicked_lobby_address_add ), NULL, this );
	m_button_lobby_address_del->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( NetworkSheet::on_button_clicked_lobby_address_del ), NULL, this );
	m_button_edit_comment->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( NetworkSheet::on_button_clicked_edit_comment ), NULL, this );
	
}

ColorEditSheet::ColorEditSheet( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer;
	bSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizerSpriteInfo;
	bSizerSpriteInfo = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText204 = new wxStaticText( this, wxID_ANY, wxT("キャラ"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText204->Wrap( -1 );
	bSizerSpriteInfo->Add( m_staticText204, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	wxArrayString m_choice_charaChoices;
	m_choice_chara = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 96,-1 ), m_choice_charaChoices, 0 );
	m_choice_chara->SetSelection( 0 );
	bSizerSpriteInfo->Add( m_choice_chara, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_staticText205 = new wxStaticText( this, wxID_ANY, wxT("カラー"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText205->Wrap( -1 );
	bSizerSpriteInfo->Add( m_staticText205, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	wxArrayString m_choice_colorChoices;
	m_choice_color = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 96,-1 ), m_choice_colorChoices, 0 );
	m_choice_color->SetSelection( 0 );
	bSizerSpriteInfo->Add( m_choice_color, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_staticText206 = new wxStaticText( this, wxID_ANY, wxT("ID"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText206->Wrap( -1 );
	bSizerSpriteInfo->Add( m_staticText206, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	wxArrayString m_choice_idChoices;
	m_choice_id = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 64,-1 ), m_choice_idChoices, 0 );
	m_choice_id->SetSelection( 0 );
	bSizerSpriteInfo->Add( m_choice_id, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_button_save = new wxButton( this, wxID_ANY, wxT("保存"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	bSizerSpriteInfo->Add( m_button_save, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_button_revert = new wxButton( this, wxID_ANY, wxT("デフォルトに戻す"), wxDefaultPosition, wxSize( 88,-1 ), 0 );
	bSizerSpriteInfo->Add( m_button_revert, 0, wxALL, 5 );
	
	bSizer->Add( bSizerSpriteInfo, 0, wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer->Add( m_staticline1, 0, wxEXPAND | wxALL, 2 );
	
	wxBoxSizer* bSizerCanvas;
	bSizerCanvas = new wxBoxSizer( wxHORIZONTAL );
	
	m_panel_palette = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( 65,257 ), wxTAB_TRAVERSAL );
	bSizerCanvas->Add( m_panel_palette, 0, wxALL, 2 );
	
	m_scrolledWindow_sprite = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scrolledWindow_sprite->SetScrollRate( 5, 5 );
	bSizerCanvas->Add( m_scrolledWindow_sprite, 1, wxEXPAND, 5 );
	
	bSizer->Add( bSizerCanvas, 1, wxEXPAND, 5 );
	
	this->SetSizer( bSizer );
	this->Layout();
	
	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( ColorEditSheet::on_init ) );
	m_choice_chara->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ColorEditSheet::on_choice_chara ), NULL, this );
	m_choice_color->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ColorEditSheet::on_choice_color ), NULL, this );
	m_choice_id->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ColorEditSheet::on_choice_id ), NULL, this );
	m_button_save->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ColorEditSheet::on_button_clicked_save_palette ), NULL, this );
	m_button_revert->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ColorEditSheet::on_button_clicked_revert_palette ), NULL, this );
	m_panel_palette->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( ColorEditSheet::on_left_down ), NULL, this );
	m_panel_palette->Connect( wxEVT_PAINT, wxPaintEventHandler( ColorEditSheet::on_paint_palette ), NULL, this );
	m_scrolledWindow_sprite->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( ColorEditSheet::on_left_down ), NULL, this );
	m_scrolledWindow_sprite->Connect( wxEVT_PAINT, wxPaintEventHandler( ColorEditSheet::on_paint_sprite ), NULL, this );
}

ColorEditSheet::~ColorEditSheet()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( ColorEditSheet::on_init ) );
	m_choice_chara->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ColorEditSheet::on_choice_chara ), NULL, this );
	m_choice_color->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ColorEditSheet::on_choice_color ), NULL, this );
	m_choice_id->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ColorEditSheet::on_choice_id ), NULL, this );
	m_button_save->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ColorEditSheet::on_button_clicked_save_palette ), NULL, this );
	m_button_revert->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ColorEditSheet::on_button_clicked_revert_palette ), NULL, this );
	m_panel_palette->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( ColorEditSheet::on_left_down ), NULL, this );
	m_panel_palette->Disconnect( wxEVT_PAINT, wxPaintEventHandler( ColorEditSheet::on_paint_palette ), NULL, this );
	m_scrolledWindow_sprite->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( ColorEditSheet::on_left_down ), NULL, this );
	m_scrolledWindow_sprite->Disconnect( wxEVT_PAINT, wxPaintEventHandler( ColorEditSheet::on_paint_sprite ), NULL, this );
	
}

EditCommentDialog::EditCommentDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer73;
	bSizer73 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer74;
	bSizer74 = new wxBoxSizer( wxHORIZONTAL );
	
	m_textCtrl_comment = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_NO_VSCROLL );
	bSizer74->Add( m_textCtrl_comment, 1, wxALL|wxEXPAND, 2 );
	
	bSizer73->Add( bSizer74, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer75;
	bSizer75 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText207 = new wxStaticText( this, wxID_ANY, wxT("文字数"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText207->Wrap( -1 );
	bSizer75->Add( m_staticText207, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_staticText_char_count = new wxStaticText( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_char_count->Wrap( -1 );
	bSizer75->Add( m_staticText_char_count, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_staticText2071 = new wxStaticText( this, wxID_ANY, wxT("/"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2071->Wrap( -1 );
	bSizer75->Add( m_staticText2071, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_staticText_char_max = new wxStaticText( this, wxID_ANY, wxT("255"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_char_max->Wrap( -1 );
	bSizer75->Add( m_staticText_char_max, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	m_sdbSizer7 = new wxStdDialogButtonSizer();
	m_sdbSizer7OK = new wxButton( this, wxID_OK );
	m_sdbSizer7->AddButton( m_sdbSizer7OK );
	m_sdbSizer7Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer7->AddButton( m_sdbSizer7Cancel );
	m_sdbSizer7->Realize();
	bSizer75->Add( m_sdbSizer7, 1, wxALL|wxEXPAND, 5 );
	
	bSizer73->Add( bSizer75, 0, wxEXPAND, 5 );
	
	this->SetSizer( bSizer73 );
	this->Layout();
	
	// Connect Events
	m_sdbSizer7Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( EditCommentDialog::on_cancel ), NULL, this );
	m_sdbSizer7OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( EditCommentDialog::on_ok ), NULL, this );
}

EditCommentDialog::~EditCommentDialog()
{
	// Disconnect Events
	m_sdbSizer7Cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( EditCommentDialog::on_cancel ), NULL, this );
	m_sdbSizer7OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( EditCommentDialog::on_ok ), NULL, this );
	
}

RGBEditDialog::RGBEditDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	
	wxBoxSizer* bSizer76;
	bSizer76 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizerR;
	bSizerR = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticTextR = new wxStaticText( this, wxID_ANY, wxT("R"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextR->Wrap( -1 );
	bSizerR->Add( m_staticTextR, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_sliderR = new wxSlider( this, wxID_ANY, 50, 0, 255, wxDefaultPosition, wxSize( -1,-1 ), wxSL_HORIZONTAL );
	bSizerR->Add( m_sliderR, 1, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlR = new wxTextCtrl( this, wxID_ANY, wxT("255"), wxDefaultPosition, wxSize( 32,-1 ), 0 );
	bSizerR->Add( m_textCtrlR, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	bSizer34->Add( bSizerR, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerG;
	bSizerG = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticTextG = new wxStaticText( this, wxID_ANY, wxT("G"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextG->Wrap( -1 );
	bSizerG->Add( m_staticTextG, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_sliderG = new wxSlider( this, wxID_ANY, 50, 0, 255, wxDefaultPosition, wxSize( 256,-1 ), wxSL_HORIZONTAL );
	bSizerG->Add( m_sliderG, 1, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlG = new wxTextCtrl( this, wxID_ANY, wxT("255"), wxDefaultPosition, wxSize( 32,-1 ), 0 );
	bSizerG->Add( m_textCtrlG, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	bSizer34->Add( bSizerG, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerB;
	bSizerB = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticTextB = new wxStaticText( this, wxID_ANY, wxT("B"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextB->Wrap( -1 );
	bSizerB->Add( m_staticTextB, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_sliderB = new wxSlider( this, wxID_ANY, 50, 0, 255, wxDefaultPosition, wxSize( -1,-1 ), wxSL_HORIZONTAL );
	bSizerB->Add( m_sliderB, 1, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	m_textCtrlB = new wxTextCtrl( this, wxID_ANY, wxT("255"), wxDefaultPosition, wxSize( 32,-1 ), 0 );
	bSizerB->Add( m_textCtrlB, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	bSizer34->Add( bSizerB, 0, wxEXPAND, 5 );
	
	bSizer35->Add( bSizer34, 1, wxEXPAND, 5 );
	
	m_colourPicker = new wxColourPickerCtrl( this, wxID_ANY, *wxBLACK, wxDefaultPosition, wxSize( -1,-1 ), wxCLRP_DEFAULT_STYLE );
	m_colourPicker->SetMinSize( wxSize( 32,-1 ) );
	
	bSizer35->Add( m_colourPicker, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
	
	bSizer76->Add( bSizer35, 1, wxEXPAND, 5 );
	
	m_sdbSizer8 = new wxStdDialogButtonSizer();
	m_sdbSizer8OK = new wxButton( this, wxID_OK );
	m_sdbSizer8->AddButton( m_sdbSizer8OK );
	m_sdbSizer8Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer8->AddButton( m_sdbSizer8Cancel );
	m_sdbSizer8->Realize();
	bSizer76->Add( m_sdbSizer8, 0, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( bSizer76 );
	this->Layout();
	
	// Connect Events
	m_sliderR->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_textCtrlR->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( RGBEditDialog::on_text ), NULL, this );
	m_sliderG->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_textCtrlG->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( RGBEditDialog::on_text ), NULL, this );
	m_sliderB->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_textCtrlB->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( RGBEditDialog::on_text ), NULL, this );
	m_colourPicker->Connect( wxEVT_COMMAND_COLOURPICKER_CHANGED, wxColourPickerEventHandler( RGBEditDialog::on_picked ), NULL, this );
	m_sdbSizer8Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RGBEditDialog::on_cancel ), NULL, this );
	m_sdbSizer8OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RGBEditDialog::on_ok ), NULL, this );
}

RGBEditDialog::~RGBEditDialog()
{
	// Disconnect Events
	m_sliderR->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderR->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_textCtrlR->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( RGBEditDialog::on_text ), NULL, this );
	m_sliderG->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderG->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_textCtrlG->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( RGBEditDialog::on_text ), NULL, this );
	m_sliderB->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_sliderB->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( RGBEditDialog::on_scroll ), NULL, this );
	m_textCtrlB->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( RGBEditDialog::on_text ), NULL, this );
	m_colourPicker->Disconnect( wxEVT_COMMAND_COLOURPICKER_CHANGED, wxColourPickerEventHandler( RGBEditDialog::on_picked ), NULL, this );
	m_sdbSizer8Cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RGBEditDialog::on_cancel ), NULL, this );
	m_sdbSizer8OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RGBEditDialog::on_ok ), NULL, this );
	
}
