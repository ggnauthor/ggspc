///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __ggscfg_gui__
#define __ggscfg_gui__

class ggscfg_ColorEditSheet;
class ggscfg_GeneralSheet;
class ggscfg_NetworkSheet;

#include <wx/panel.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/notebook.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/statline.h>
#include <wx/scrolwin.h>
#include <wx/clrpicker.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class ConfigDialog
///////////////////////////////////////////////////////////////////////////////
class ConfigDialog : public wxDialog 
{
	private:
	
	protected:
		wxNotebook* m_notebook;
		ggscfg_GeneralSheet* m_panel_general;
		ggscfg_NetworkSheet* m_panel_network;
		ggscfg_ColorEditSheet* m_panel_color_edit;
		
		wxButton* m_button_ok;
		wxButton* m_button_cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void on_ok( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_cancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		ConfigDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("ggspc config"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 580,480 ), long style = wxCAPTION|wxDIALOG_NO_PARENT|wxRESIZE_BORDER );
		~ConfigDialog();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class GeneralSheet
///////////////////////////////////////////////////////////////////////////////
class GeneralSheet : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_staticText11;
		wxChoice* m_choice_display_device;
		wxStaticText* m_staticText111;
		wxChoice* m_choice_display_mode;
		wxStaticText* m_staticText1;
		wxChoice* m_choice_color_depth;
		wxCheckBox* m_checkBox_wait_vsync;
		wxCheckBox* m_checkBox_frame_skip;
		wxCheckBox* m_checkBox_keep_aspect_ratio;
		wxStaticText* m_staticText_bgm;
		wxCheckBox* m_checkBox_bgm;
		wxSlider* m_slider_bgm;
		wxTextCtrl* m_textCtrl_bgm;
		wxStaticText* m_staticText_se;
		wxCheckBox* m_checkBox_se;
		wxSlider* m_slider_se;
		wxTextCtrl* m_textCtrl_se;
		wxStaticText* m_staticText_voice;
		wxCheckBox* m_checkBox_voice;
		wxSlider* m_slider_voice;
		wxTextCtrl* m_textCtrl_voice;
		wxCheckBox* m_checkBox_play_background;
		wxCheckBox* m_checkBox_display_fps;
		wxCheckBox* m_checkBox_disable_movie;
		wxCheckBox* m_checkBox_disable_screen_saver;
		wxCheckBox* m_checkBox_input_background;
		wxCheckBox* m_checkBox_unlock_all_contents;
		wxStaticText* m_staticText81;
		wxChoice* m_choice_device1P;
		wxStaticText* m_staticText64;
		wxSpinCtrl* m_spin_analog_sense_1P;
		wxStaticText* m_staticText18;
		wxTextCtrl* m_textCtrlUp_1P;
		wxStaticText* m_staticText182;
		wxTextCtrl* m_textCtrlDown_1P;
		wxStaticText* m_staticText183;
		wxTextCtrl* m_textCtrlLeft_1P;
		wxStaticText* m_staticText184;
		wxTextCtrl* m_textCtrlRight_1P;
		wxStaticText* m_staticText185;
		wxTextCtrl* m_textCtrlCircle_1P;
		wxStaticText* m_staticText1851;
		wxTextCtrl* m_textCtrlCross_1P;
		wxStaticText* m_staticText1852;
		wxTextCtrl* m_textCtrlTriangle_1P;
		wxStaticText* m_staticText1853;
		wxTextCtrl* m_textCtrlSquare_1P;
		wxStaticText* m_staticText1854;
		wxTextCtrl* m_textCtrlL1_1P;
		wxStaticText* m_staticText1856;
		wxTextCtrl* m_textCtrlR1_1P;
		wxStaticText* m_staticText1855;
		wxTextCtrl* m_textCtrlL2_1P;
		wxStaticText* m_staticText1857;
		wxTextCtrl* m_textCtrlR2_1P;
		wxStaticText* m_staticText18541;
		wxTextCtrl* m_textCtrlL3_1P;
		wxStaticText* m_staticText18542;
		wxTextCtrl* m_textCtrlR3_1P;
		wxStaticText* m_staticText185412;
		wxTextCtrl* m_textCtrlStart_1P;
		wxStaticText* m_staticText185413;
		wxTextCtrl* m_textCtrlSelect_1P;
		wxStaticText* m_staticText811;
		wxChoice* m_choice_device2P;
		wxStaticText* m_staticText641;
		wxSpinCtrl* m_spin_analog_sense_2P;
		wxStaticText* m_staticText181;
		wxTextCtrl* m_textCtrlUp_2P;
		wxStaticText* m_staticText1821;
		wxTextCtrl* m_textCtrlDown_2P;
		wxStaticText* m_staticText1831;
		wxTextCtrl* m_textCtrlLeft_2P;
		wxStaticText* m_staticText1841;
		wxTextCtrl* m_textCtrlRight_2P;
		wxStaticText* m_staticText1858;
		wxTextCtrl* m_textCtrlCircle_2P;
		wxStaticText* m_staticText18511;
		wxTextCtrl* m_textCtrlCross_2P;
		wxStaticText* m_staticText18521;
		wxTextCtrl* m_textCtrlTriangle_2P;
		wxStaticText* m_staticText18531;
		wxTextCtrl* m_textCtrlSquare_2P;
		wxStaticText* m_staticText18543;
		wxTextCtrl* m_textCtrlL1_2P;
		wxStaticText* m_staticText18561;
		wxTextCtrl* m_textCtrlR1_2P;
		wxStaticText* m_staticText18551;
		wxTextCtrl* m_textCtrlL2_2P;
		wxStaticText* m_staticText18571;
		wxTextCtrl* m_textCtrlR2_2P;
		wxStaticText* m_staticText185414;
		wxTextCtrl* m_textCtrlL3_2P;
		wxStaticText* m_staticText1854141;
		wxTextCtrl* m_textCtrlR3_2P;
		wxStaticText* m_staticText185411;
		wxTextCtrl* m_textCtrlStart_2P;
		wxStaticText* m_staticText185421;
		wxTextCtrl* m_textCtrlSelect_2P;
		wxTextCtrl* m_textCtrl_dummy;
		
		// Virtual event handlers, overide them in your derived class
		virtual void on_init( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void on_left_down( wxMouseEvent& event ) { event.Skip(); }
		virtual void onchoice_screen_mode( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_scroll_volume( wxScrollEvent& event ) { event.Skip(); }
		virtual void on_text_volume( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_choice_device( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		GeneralSheet( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
		~GeneralSheet();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class NetworkSheet
///////////////////////////////////////////////////////////////////////////////
class NetworkSheet : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_staticText_port;
		wxTextCtrl* m_textCtrl_port;
		wxStaticText* m_staticText_delay;
		wxChoice* m_choice_delay;
		wxStaticText* m_staticText_rollback;
		wxChoice* m_choice_rollback;
		wxCheckBox* m_checkBox_ex;
		wxCheckBox* m_checkBox_cheat;
		wxStaticText* m_staticText_round;
		wxChoice* m_choice_round;
		wxCheckBox* m_checkBox_allow_watch;
		wxCheckBox* m_checkBox_allow_intrusion;
		wxCheckBox* m_checkBox_save_replay;
		wxStaticText* m_staticText_max_relay;
		wxChoice* m_choice_max_relay;
		wxStaticText* m_staticText2031;
		wxComboBox* m_comboBox_lobby_address;
		wxButton* m_button_lobby_address_add;
		wxButton* m_button_lobby_address_del;
		wxStaticText* m_staticText1992;
		wxComboBox* m_comboBox_font_face;
		wxStaticText* m_staticText19911;
		wxTextCtrl* m_textCtrl_font_size;
		wxCheckBox* m_checkBox_antialias;
		wxStaticText* m_staticText86;
		wxChoice* m_choice_deny_type;
		wxCheckBox* m_checkBox_deny_slow;
		wxCheckBox* m_checkBox_deny_disconnect;
		wxStaticText* m_staticText_player_name;
		wxTextCtrl* m_textCtrl_player_name;
		wxStaticText* m_staticText_tripkey;
		wxTextCtrl* m_textCtrl_tripkey;
		wxStaticText* m_staticText_rank;
		wxTextCtrl* m_textCtrl_rank;
		wxStaticText* m_staticText_win_rate;
		wxTextCtrl* m_textCtrl_win_rate;
		wxStaticText* m_staticText_win_count;
		wxTextCtrl* m_textCtrl_win_count;
		wxStaticText* m_staticText_slow_rate;
		wxTextCtrl* m_textCtrl_slow_rate;
		wxStaticText* m_staticText_game_count;
		wxTextCtrl* m_textCtrl_game_count;
		wxStaticText* m_staticText_disconnect_rate;
		wxTextCtrl* m_textCtrl_disconnect_rate;
		wxButton* m_button_edit_comment;
		
		// Virtual event handlers, overide them in your derived class
		virtual void on_init( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void on_button_clicked_lobby_address_add( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_button_clicked_lobby_address_del( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_button_clicked_edit_comment( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		NetworkSheet( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
		~NetworkSheet();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ColorEditSheet
///////////////////////////////////////////////////////////////////////////////
class ColorEditSheet : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_staticText204;
		wxChoice* m_choice_chara;
		wxStaticText* m_staticText205;
		wxChoice* m_choice_color;
		wxStaticText* m_staticText206;
		wxChoice* m_choice_id;
		wxButton* m_button_save;
		wxButton* m_button_revert;
		wxStaticLine* m_staticline1;
		wxPanel* m_panel_palette;
		wxScrolledWindow* m_scrolledWindow_sprite;
		
		// Virtual event handlers, overide them in your derived class
		virtual void on_init( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void on_choice_chara( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_choice_color( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_choice_id( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_button_clicked_save_palette( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_button_clicked_revert_palette( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_left_down( wxMouseEvent& event ) { event.Skip(); }
		virtual void on_paint_palette( wxPaintEvent& event ) { event.Skip(); }
		virtual void on_paint_sprite( wxPaintEvent& event ) { event.Skip(); }
		
	
	public:
		
		ColorEditSheet( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 521,340 ), long style = wxTAB_TRAVERSAL );
		~ColorEditSheet();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class EditCommentDialog
///////////////////////////////////////////////////////////////////////////////
class EditCommentDialog : public wxDialog 
{
	private:
	
	protected:
		wxTextCtrl* m_textCtrl_comment;
		wxStaticText* m_staticText207;
		wxStaticText* m_staticText_char_count;
		wxStaticText* m_staticText2071;
		wxStaticText* m_staticText_char_max;
		wxStdDialogButtonSizer* m_sdbSizer7;
		wxButton* m_sdbSizer7OK;
		wxButton* m_sdbSizer7Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void on_cancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_ok( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		EditCommentDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Edit Comment"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 400,140 ), long style = wxDEFAULT_DIALOG_STYLE );
		~EditCommentDialog();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RGBEditDialog
///////////////////////////////////////////////////////////////////////////////
class RGBEditDialog : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticTextR;
		wxSlider* m_sliderR;
		wxTextCtrl* m_textCtrlR;
		wxStaticText* m_staticTextG;
		wxSlider* m_sliderG;
		wxTextCtrl* m_textCtrlG;
		wxStaticText* m_staticTextB;
		wxSlider* m_sliderB;
		wxTextCtrl* m_textCtrlB;
		wxColourPickerCtrl* m_colourPicker;
		wxStdDialogButtonSizer* m_sdbSizer8;
		wxButton* m_sdbSizer8OK;
		wxButton* m_sdbSizer8Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void on_scroll( wxScrollEvent& event ) { event.Skip(); }
		virtual void on_text( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_picked( wxColourPickerEvent& event ) { event.Skip(); }
		virtual void on_cancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_ok( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		RGBEditDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("RGB Edit"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 250,150 ), long style = wxCAPTION );
		~RGBEditDialog();
	
};

#endif //__ggscfg_gui__
