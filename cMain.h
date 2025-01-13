#pragma once

#include "wx/wx.h"

class cMain : public wxFrame
{
public:
	cMain();
	~cMain();

public:
	wxButton* m_btn1 = nullptr;
	wxTextCtrl* m_txt1 = nullptr;
	wxListBox* m_list1 = nullptr;

	wxChoice* m_choice = nullptr;     // Dropdown menu
	wxButton* m_btnAdd = nullptr;     // "+" Button
	wxListBox* m_listBox = nullptr;   // Middle box for displaying options
	wxString m_selectedFilePath;

	void OnButtonClicked(wxCommandEvent& evt);

	wxDECLARE_EVENT_TABLE();
};

