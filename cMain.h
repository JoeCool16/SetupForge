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
	wxButton* m_btnSave = nullptr;
	wxButton* m_btnDelete = nullptr;
	wxButton* m_btnSaveListbox = nullptr;
	wxButton* m_btnOpenListbox = nullptr;
	wxString m_selectedFilePath;

	void OnButtonClicked(wxCommandEvent& evt);
	void OnRunScriptClicked(wxCommandEvent& evt);
	void OnSaveListboxClicked(wxCommandEvent& evt);
	void OnOpenListboxClicked(wxCommandEvent& evt);
	void OnDeleteButtonClicked(wxCommandEvent& evt);

	// Utility functions for actions
	void RunExe(const wxString& exePath);
	void FileMover(const wxString& sourcePath, const wxString& destinationPath);
	void CreateFolder(const wxString& folderPath);
	void CreateFiles(const wxString& filePath);
	void SetEV(const wxString& varName, const wxString& varValue, bool append);

	wxDECLARE_EVENT_TABLE();
};

