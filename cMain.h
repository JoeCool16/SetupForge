#pragma once

#include "wx/wx.h"
#include "resource.h"

class cMain : public wxFrame
{
public:
	cMain();
	~cMain();

private:
	wxStaticText* m_stepLabel = nullptr;  // Step progress label
	int m_draggedIndex = wxNOT_FOUND;
	wxString m_scriptDirectory;
	int m_tempErrorPointIndex = -1;  // Stores the index of the error point, -1 means no error
	int m_tempRestartIndex = -1;
	bool m_errorFlag = false;

public:
	wxButton* m_btn1 = nullptr;
	wxTextCtrl* m_txt1 = nullptr;
	wxListBox* m_list1 = nullptr;
	wxGauge* m_progressBar = nullptr;
	wxChoice* m_choice = nullptr;     // Dropdown menu
	wxButton* m_btnAdd = nullptr;     // "+" Button
	wxListBox* m_listBox = nullptr;   // Middle box for displaying options
	wxButton* m_btnSave = nullptr;
	wxButton* m_btnDelete = nullptr;
	wxButton* m_btnSaveListbox = nullptr;
	wxButton* m_btnOpenListbox = nullptr;
	wxButton* m_btnClearAll = nullptr;
	wxString m_selectedFilePath;
	wxArrayString choices;

	void OnButtonClicked(wxCommandEvent& evt);
	void OnRunScriptClicked(wxCommandEvent& evt);
	void OnSaveListboxClicked(wxCommandEvent& evt);
	void OnOpenListboxClicked(wxCommandEvent& evt);
	void OnDeleteButtonClicked(wxCommandEvent& evt);
	void OnClearAllButtonClicked(wxCommandEvent& evt);
	void OnListBoxDoubleClick(wxCommandEvent& evt);
	void OnListBoxMouseDown(wxMouseEvent& event);
	void OnListBoxMouseMove(wxMouseEvent& event);
	void OnListBoxMouseUp(wxMouseEvent& event);
	void WriteToLog(const wxString& message);

	// Utility functions for actions
	void RunExe(const wxString& exePath);
	void FileMover(const wxString& sourcePath, const wxString& destinationPath);
	void FolderMover(const wxString& sourcePath, const wxString& destinationPath);
	bool DeleteFolderRecursively(const wxString& folderPath);
	void CreateFolder(const wxString& folderPath);
	void CreateFiles(const wxString& filePath);
	void SetEV(const wxString& varName, const wxString& varValue, bool append);
	void SetSystemEV(const wxString& varName, const wxString& varValue, bool append);
	bool ModifyRegistry(const wxString& keyPath, const wxString& valueName, const wxString& valueData);
	bool MapNetworkDrive(const wxString& driveLetter, const wxString& networkPath, const wxString& persistentFlag);

	wxDECLARE_EVENT_TABLE();
};

